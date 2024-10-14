**Copyright 2023 Dumitrescu Toma-Ioan**


### Load Balancer

### Description:

* This program is used for efficiently and uniforming storing different
products on a set of servers. Possible operations: add product, add server,
remove server.

* Command list to run the program:
make
./tema2 input_file.txt

* Linking:

We have the headers utils.h, list.h and hashtable.h, with a relation of
inclusion in this order. The header server.h is connected to utils.h (for
DIE macro) and hashtable.h (for its operations). The header load_balancer.h
is connected to utils, list (for object redistribution), hashtable and
server (for server_memory data type). If we have X.c, then X.c uses X.h,
except main which is connected to load_balancer and utils.

* Load balancer:

For simple freeing, load balancer retain variable size and nr_servers (useful
to know the difference between the actual size and its capacity, for realloc).
An array of structures of type hashring, where all servers, hashes and indexes
are retained. The structure was updated so that sorting, interogation and
memory manipulation were easier. The array of hashring is decreasingly sorted
so that all 0 are shifted to right, remaining the segment 0, nr_servers-1 to
work on. All replicas are added to the hashring. When adding/removing a server,
it is sufficiently to check if redistribution is need, only for all objects of the
next server, following circularity principle of the hashring. Some functions were
decomposed in more function, to make the code more legible and to make more clear
the algorithms that were used. For main operations, some functions were called
three times, at each step server_id being incremented with POW_HASH, the constant
to determine the replicas indexes. Rule of determining the server for an object:
the first server for which object_hash <= server_hash or the server with the smallest
hash.

* Memory:

Although intuition tells that the program will crash because of too much memory
allocation, a C program will still work after creating a double array of 1.5 million
size. So, the maximum number of servers (with replicas) is in practice 60000, looking
at tests and in normal life-situations, the limit of 99999 being given only to
make the assuption of no intersections between a new_server_id and a replica from
a preexisting server. So, in the upper bound (double size is bigger), we have, neglecting
many factors, the inequality memory < 60000 * (1 + 1 + 20) + 100000 (other declarations)
= 1,42 million blocks of double. So, the program should not have problems in this area.

Freeing method: the pyramid rule (up -> base). The array of structures will be freed
as a block.

* Server implementation:

A server is retained in the memory typically as a hashtable, where products
are distributed using its hash_function. Other operations are done using the
basic hashtable functions (because of the generic implementation).

### Comments:

* Hardest point in the homework: too little time to finish
* Initially, hashring consisted in an array of indexes and an array of
hashes, but the difficulty to sort both determined the pre-existing
structure format.
* Another key point was when I called the function server_remove in
redistribute_objects. That freed the node and caused invalid read of size 8
when checking with valgrind.
* Pragmatic subject

### Resources/Bibliography:

* https://en.wikipedia.org/wiki/Consistent_hashing
