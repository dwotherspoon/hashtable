
typedef struct item_t {
	unsigned long hash;
	char * key;
	unsigned int key_len;
	void * value;
	struct item_t * pPrev;
	struct item_t * pNext;
} Item;

typedef struct entry_t {
	Item * pHead;
	Item * pTail;
} Entry;

typedef struct hashtable_t {
	/* Number of elements in array. */
	unsigned int num_elements;
	/* Size of the table. */
	unsigned int size;
	/* Table mask. */
	unsigned int mask;
	/* Entries array (main storage). */
	Entry * entries;
} HashTable;

void hashtable_init(HashTable * table, unsigned int size);
void hashtable_deinit(HashTable * table);
void hashtable_debug(HashTable * table);

void * hashtable_get(HashTable * table, const char * key, unsigned int len);
void hashtable_set(HashTable * table, const char * key, unsigned int len, void * value);
