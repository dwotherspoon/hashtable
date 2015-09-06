typedef struct entry_t {
	unsigned long hash;
	const char * key;
	unsigned int key_len;
	void * value;
	struct entry_t * pPrev;
	struct entry_t * pNext;
} Entry;

typedef struct hashtable_t {
	/* Number of elements in array */
	unsigned int num_elements;
	/* Size of the table */
	unsigned int size;
	/* Table mask */
	unsigned int mask;
	/* Pointer to entries array */
	Entry ** entries;
	void * x;
} HashTable;

void hashtable_init(HashTable * table, unsigned int size);
void hashtable_deinit(HashTable * table);
void hashtable_debug(HashTable * table);

void * hashtable_get(HashTable * table, const char * key, const unsigned int len);
void hashtable_insert(HashTable * table, const char * key, unsigned int len, void * value);