/* Copyright 2023 <Toma Dumitrescu> */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "utils.h"
#include "list.h"
#include "hashtable.h"
#include "server.h"

#define INIT_SIZE 30
#define POW_HASH 100000

typedef struct hashring_t {
	int index;
	unsigned int hash;
	server_memory *server;
} hashring_t;

typedef struct load_balancer {
	hashring_t *hashring;
	unsigned int nr_servers;
	unsigned int size;
} load_balancer;

/**
 * init_load_balancer() - initializes the memory for a new load balancer and its fields and
 *							returns a pointer to it
 *
 * Return: pointer to the load balancer struct
 */
load_balancer *init_load_balancer();

/**
 * free_load_balancer() - frees the memory of every field that is related to the
 * load balancer (servers, hashring)
 *
 * @arg1: Load balancer to free
 */
void free_load_balancer(load_balancer *main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will RETURN via this parameter
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the
 * load across the servers. The chosen server ID will be returned
 * using the last parameter.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 *
 */
void loader_store(load_balancer *main, char *key, char *value, int *server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will RETURN the server ID
          which stores the value via this parameter.
 *
 * The load balancer will search for the server which should posess the
 * value associated to the key. The server will return NULL in case
 * the key does NOT exist in the system.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 */
char *loader_retrieve(load_balancer *main, char *key, int *server_id);

/**
 * load_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 3 replica labels and it will
 * place them inside the hash ring. The neighbor servers will
 * distribute some the objects to the added server.
 *
 * Hint:
 * Resize the servers array to add a new one.
 * Add each label in the hashring in its appropiate position.
 * Do not forget to resize the hashring and redistribute the objects
 * after each label add (the operations will be done 3 times, for each replica).
 */
void loader_add_server(load_balancer *main, int server_id);

/**
 * load_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute ALL objects stored on the
 * removed server and will delete ALL replicas from the hash ring.
 *
 */
void loader_remove_server(load_balancer *main, int server_id);

/*
 * @arg1: load balancer for product distribution
 *
 * resizes the hashring by 3 * actual_size
 * because of the 3 meme-servers, size should be divisible by 3
 */
void loader_resize(load_balancer *main);

/*
 * @arg1: load balancer for uniform division of objects
 * @arg2: the id of the server where the operation is performed
 *
 * add the replica server to the hashring
 */
int add_replica(load_balancer *main, int server_id);

/*
 * @arg1: load balancer for object distributions
 * @arg2: the index of the target server in the (sorted) hashring
 *
 * remaps the objects from right server
 */
void redistribute_objects_add(load_balancer *main, int idx);

/*
 * @arg1: load balancer for task classification
 * @arg2: the id of the server that should be deleted
 *
 * removes the replica server from the hashring
 */
void remove_replica(load_balancer *main, int server_id);

/*
 * @arg1: load balancer for uniform distribution of products
 * @arg2: the id of the target server in the (sorted) hashring
 *
 * remaps the objects from the actual server
 */
void redistribute_objects_rm(load_balancer *main, int idx);

/*
 * @arg1: load balancer for equilibrated dispersal
 * @arg2, 3: index boundes for the array where function searches
 * @arg4: value that is searched
 *
 * performs binary search on a decreasingly sorted array
 */
int d_binary_search(load_balancer *main, int left, int right,
				unsigned int val);

#endif /* LOAD_BALANCER_H_ */
