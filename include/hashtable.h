#ifndef HASHTABLE_H
#define HASHTABLE_H 

#include <stdint.h>

typedef struct entry_t {
	uint64_t hash;
	char *key;
	uint32_t key_len;
	void *value;
	struct entry_t *pPrev;
	struct entry_t *pNext;
} Entry;

typedef struct hashtable_t {
	/* Number of elements in array. */
	uint32_t count;
	/* Size of the table. */
	uint32_t size;
	/* Table mask. */
	uint32_t mask;
	/* Entries array (main storage). */
	Entry **entries;

	/* Pointer to current Entry (for iterator). */
	Entry *pEntry;
	/* Current position in entries (for iterator). */
	uint32_t entries_pos;
} HashTable;

void hashtable_init(HashTable *table, uint32_t size);
void hashtable_free(HashTable *table, void (*free_func)(void *));
void hashtable_debug(HashTable *table);

void * hashtable_get(HashTable *table, const char *key, uint32_t len);
void hashtable_set(HashTable *table, const char *key, uint32_t len, void *value);
void * hashtable_unset(HashTable *table, const char *key, uint32_t len);

void hashtable_iter_first(HashTable *table);
int hashtable_iter_next(HashTable *table);

char * hashtable_iter_key(HashTable *table);
unsigned int hashtable_iter_key_len(HashTable *table);
void * hashtable_iter_value(HashTable *table);

#ifdef HASHTABLE_DBG
	#define DBGF(...) printf(__VA_ARGS__)
	#define DBG(S) puts(S)
#else
	#define DBGF(...) /* Nothing */
	#define DBG(S) /* Nothing */
#endif

#endif
