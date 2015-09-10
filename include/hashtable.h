typedef struct entry_t {
	uint64_t hash;
	char * key;
	uint32_t key_len;
	void * value;
	struct entry_t * pPrev;
	struct entry_t * pNext;
} Entry;

typedef struct hashtable_t {
	/* Number of elements in array. */
	uint32_t count;
	/* Size of the table. */
	uint32_t size;
	/* Table mask. */
	uint32_t mask;
	/* Entries array (main storage). */
	Entry ** entries;

	/* Pointer to current Entry (for iterator). */
	Entry * pEntry;
	/* Current position in entries (for iterator). */
	uint32_t pos_entries;
} HashTable;

void hashtable_init(HashTable * table, uint32_t size);
void hashtable_deinit(HashTable * table);
void hashtable_debug(HashTable * table);

void * hashtable_get(HashTable * table, const char * key, uint32_t len);
void hashtable_set(HashTable * table, const char * key, uint32_t len, void * value);
void * hashtable_unset(HashTable * table, const char * key, uint32_t len);
