/* Copyright 2023 <Toma Dumitrescu> */
#include "load_balancer.h"

unsigned int hash_function_servers(void *a)
{
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a)
{
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;
	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

load_balancer *init_load_balancer()
{
	load_balancer *main = malloc(sizeof(load_balancer));
	DIE(!main, "Malloc failed!\n");
	main->hashring = calloc(INIT_SIZE, sizeof(hashring_t));
	DIE(!main->hashring, "Calloc failed!\n");
	main->nr_servers = 0;
	main->size = INIT_SIZE;

	return main;
}

void loader_resize(load_balancer *main)
{
	main->size = 3 * main->size;
	hashring_t *tmp = realloc(main->hashring, main->size *
							  sizeof(hashring_t));
	DIE(!tmp, "Realloc failed!\n");
	main->hashring = tmp;
}

int add_replica(load_balancer *main, int server_id)
{
	unsigned int hash = hash_function_servers((void *)&server_id);
	// due to circularity reasons
	int idx = main->nr_servers;
	for (int i = 0; i < main->nr_servers; i++)
		if (hash > main->hashring[i].hash) {
			idx = i;
			// classical algorithm for element insertion in array
			for (int j = main->nr_servers; j >= i + 1; j--)
				main->hashring[j] = main->hashring[j - 1];
			break;
		}

	main->hashring[idx].index = server_id;
	main->hashring[idx].hash = hash;
	main->hashring[idx].server = init_server_memory();
	main->nr_servers++;

	return idx;
}

void redistribute_objects_add(load_balancer *main, int idx)
{
	int next_idx = idx - 1;
	if (next_idx < 0)
		next_idx = main->nr_servers - 1;

	server_memory *act_server = main->hashring[idx].server;
	server_memory *next_server = main->hashring[next_idx].server;
	for (int i = 0; i < next_server->ht->hmax; i++) {
		ll_node_t *node = next_server->ht->buckets[i]->head;
		while (node) {
			info* data_info = (info *)node->data;
			if (data_info->key) {
				unsigned int hash = hash_function_key(data_info->key);
				if (hash <= main->hashring[idx].hash ||
					hash > main->hashring[next_idx].hash) {
					server_store(act_server, (char *)data_info->key,
								 (char *)data_info->value);
				}
			}
			node = node->next;
		}
	}
}

void loader_add_server(load_balancer *main, int server_id)
{
	// check if we can add 3 more servers (1 + 2 replicas)
	if (main->nr_servers + 3 > main->size)
		loader_resize(main);

	unsigned int idx;
	idx = add_replica(main, server_id);
	redistribute_objects_add(main, idx);
	server_id += POW_HASH;
	idx = add_replica(main, server_id);
	redistribute_objects_add(main, idx);
	server_id += POW_HASH;
	idx = add_replica(main, server_id);
	redistribute_objects_add(main, idx);
}

void remove_replica(load_balancer *main, int server_id)
{
	unsigned int hash = hash_function_servers((void *)&server_id);
	// find the index using a log2 algorithm
	int idx = d_binary_search(main, 0, main->nr_servers - 1, hash);
	if (idx == -1)
		return;

	redistribute_objects_rm(main, idx);
	free_server_memory(main->hashring[idx].server);
	// classical algorithm for deleting an element in an array
	for (int i = idx; i < main->nr_servers - 1; i++)
		main->hashring[i] = main->hashring[i + 1];

	main->nr_servers--;
}

void redistribute_objects_rm(load_balancer *main, int idx)
{
	// circularity reasons
	int next_idx = idx - 1;
	if (next_idx < 0)
		next_idx = main->nr_servers - 1;

	server_memory *act_server = main->hashring[idx].server;
	server_memory *next_server = main->hashring[next_idx].server;
	for (int i = 0; i < act_server->ht->hmax; i++) {
		ll_node_t *node = act_server->ht->buckets[i]->head;
		// all elements of act_server to the next one
		while (node) {
			info* data_info = (info *)node->data;
			server_store(next_server, (char *)data_info->key,
						 (char *)data_info->value);
			node = node->next;
		}
	}
}

void loader_remove_server(load_balancer *main, int server_id)
{
	remove_replica(main, server_id);
	server_id += POW_HASH;
	remove_replica(main, server_id);
	server_id += POW_HASH;
	remove_replica(main, server_id);
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	unsigned int hash = hash_function_key((void *)key);
	*server_id = -1;
	int idx;
	for (int i = main->nr_servers - 1; i >= 0; i--)
		if (hash < main->hashring[i].hash) {
			*server_id = main->hashring[i].index;
			idx = i;
			break;
		}

	if (*server_id == -1) {
		idx = main->nr_servers - 1;
		*server_id = main->hashring[idx].index;
	}

	*server_id %= POW_HASH;
	server_store(main->hashring[idx].server, key, value);
}

int d_binary_search(load_balancer *main, int left, int right,
				   unsigned int val)
{
	if (left > right)
		return -1;

	int mid = left + (right - left) / 2;
	if (main->hashring[mid].hash == val)
		return mid;

	if (main->hashring[mid].hash < val)
		return d_binary_search(main, left, mid - 1, val);

	return d_binary_search(main, mid + 1, right, val);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id) {
	unsigned int hash = hash_function_key((void *)key);

	int idx = main->nr_servers - 1;
	for (int i = main->nr_servers - 1; i >= 0; i--)
		if (hash < main->hashring[i].hash) {
			idx = i;
			break;
		}

	*server_id = main->hashring[idx].index % POW_HASH;
	return server_retrieve(main->hashring[idx].server, key);
}

void free_load_balancer(load_balancer *main)
{
	for (int i = 0; i < main->nr_servers; i++)
		free_server_memory(main->hashring[i].server);

	free(main->hashring);
	free(main);
}
