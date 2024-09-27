/* Copyright 2023 <Toma Dumitrescu> */
#include "server.h"

server_memory *init_server_memory()
{
	server_memory *server = malloc(sizeof(server_memory));
	DIE(!server, "Malloc failed!\n");
	server->ht = ht_create(HMAX, hash_function_string,
						   compare_function_strings);

	return server;
}

void server_store(server_memory *server, char *key, char *value) {
	// key_size = strlen(key) + 1
	ht_put(server->ht, (void *)key, strlen(key) + 1,
		   (void *)value, strlen(value) + 1);
}

char *server_retrieve(server_memory *server, char *key) {
	void* obj = ht_get(server->ht, (void *)key);
	return (char *)obj;
}

void server_remove(server_memory *server, char *key) {
	ht_remove_entry(server->ht, (void *)key);
}

void free_server_memory(server_memory *server)
{
	if (server) {
		ht_free(server->ht);
		free(server);
	}
}
