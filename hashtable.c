#include <hashtable.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* (Mostly PHP's unrolled implementation.)
 * DJBX33A (DJB2) (Daniel J. Bernstein, Times 33 with Addition)
 *
 * This is Daniel J. Bernstein's popular `times 33' hash function as
 * posted by him years ago on comp.lang.c. It basically uses a function
 * like ``hash(i) = hash(i-1) * 33 + str[i]''. This is one of the best
 * known hash functions for strings. Because it is both computed very
 * fast and distributes very well.
 *
 * The magic of number 33, i.e. why it works better than many other
 * constants, prime or not, has never been adequately explained by
 * anyone. So I try an explanation: if one experimentally tests all
 * multipliers between 1 and 256 (as RSE did now) one detects that even
 * numbers are not useable at all. The remaining 128 odd numbers
 * (except for the number 1) work more or less all equally well. They
 * all distribute in an acceptable way and this way fill a hash table
 * with an average percent of approx. 86%. 
 *
 * If one compares the Chi^2 values of the variants, the number 33 not
 * even has the best value. But the number 33 and a few other equally
 * good numbers like 17, 31, 63, 127 and 129 have nevertheless a great
 * advantage to the remaining numbers in the large set of possible
 * multipliers: their multiply operation can be replaced by a faster
 * operation based on just one shift plus either a single addition
 * or subtraction operation. And because a hash function has to both
 * distribute good _and_ has to be very fast to compute, those few
 * numbers should be preferred and seems to be the reason why Daniel J.
 * Bernstein also preferred it.
 *
 *
 *                  -- Ralf S. Engelschall <rse@engelschall.com>
 */

#define DJBX_ROUND(H, K) hash = (((H) << 5) + (H)) + *(K)++;

static uint64_t inline hash_djbx(const char *key, uint32_t len) {
	register uint64_t hash = 5381;

	for (;len >= 8; len -= 8) {
		DJBX_ROUND(hash, key);
		DJBX_ROUND(hash, key);
		DJBX_ROUND(hash, key);
		DJBX_ROUND(hash, key);
		DJBX_ROUND(hash, key);
		DJBX_ROUND(hash, key);
		DJBX_ROUND(hash, key);
		DJBX_ROUND(hash, key);
	}
	/* Do remaining rounds. (n.b. fallthrough) */
	switch (len) {
		case 7: DJBX_ROUND(hash, key);
		case 6: DJBX_ROUND(hash, key);
		case 5: DJBX_ROUND(hash, key);
		case 4: DJBX_ROUND(hash, key);
		case 3: DJBX_ROUND(hash, key);
		case 2: DJBX_ROUND(hash, key);
		case 1: DJBX_ROUND(hash, key);
		case 0: break;
	}
	return hash;
} 

/* Define so we can switch hash funcs easily.*/
#define HASH(k, l) hash_djbx(k, l)

void hashtable_init(HashTable *table, uint32_t size) {
	/* Find power of two greater than size. Min size is 8 elems. */
	uint32_t i = 3;

	for (;((uint32_t)1 << i) < size; i++);
	table->count = 0;
	table->size = 1 << i;
	table->mask = table->size - 1;
	printf("Init: HashTable with size %u (requested %u).\n", table->size, size);
	table->entries = calloc(table->size, sizeof(Entry *));
}

void hashtable_free(HashTable *table, void (*free_func)(void *)) {
	uint32_t pos = 0;
	Entry *pEntry, *pNext;

	for (; pos < table->size; pos++) {
		pEntry = table->entries[pos];
		for (; pEntry; pEntry = pNext) {
			pNext = pEntry->pNext;
			free(pEntry->key);
			/* Ignore free_func if NULL. */
			if (free_func) {
				(*free_func)(pEntry->value);
			}
			free(pEntry);
		}
	}
	/* Free entries */
	free(table->entries);
}

static void hashtable_resize(HashTable *table) {
	Entry **new_entries;
	Entry *pEntry, *pNext, *pTail;
	uint32_t pos = 0;
	uint32_t new_size = table->size << 1;
	uint32_t new_mask = new_size - 1;

	printf("Resize: Hashtable resized to %u.\n", new_size);
	/* PHP handles this using realloc as they have a linked list
		 over all entries to maintain order. */
	new_entries = calloc(new_size, sizeof(Entry *));
	/* Reindex (TODO: possible code repetition with set) */
	for (;pos < table->size; pos++) {
		pEntry = table->entries[pos];
		while (pEntry) {
			pNext = pEntry->pNext;
			if (new_entries[pEntry->hash & new_mask]) {
				pTail = new_entries[pEntry->hash & new_mask];
				/* Find the tail */
				while (pTail->pNext) {
					pTail = pTail->pNext;
				}
				pTail->pNext = pEntry;
				pEntry->pPrev = pTail;
				pEntry->pNext = NULL;
			}
			else {
				/* Item is to be head in new entries */
				new_entries[pEntry->hash & new_mask] = pEntry;
				pEntry->pPrev = NULL;
				pEntry->pNext = NULL;
			}
			pEntry = pNext;
		}
	}
	/* Free what we had and update struct. */
	free(table->entries);
	table->entries = new_entries;
	table->size = new_size;
	table->mask = new_mask;
}

void hashtable_debug(HashTable *table) {
	uint32_t i;
	Entry *pEntry;

	for (i = 0; i < table->size; i++) {
		printf("%i => ", i);
		pEntry = table->entries[i];
		for (; pEntry; pEntry = pEntry->pNext) {		
			printf("%p, ", pEntry);
		} 
		puts(" NULL.");
	}
}

/* Test an entry, with full hash, key and length. */
static int inline hashtable_entry_test(Entry *pEntry, uint64_t hash, const char *key, uint32_t len) {
	if (pEntry == NULL) {
		/* Always false for null items. */
		return 0;
	}
	if ((pEntry->hash != hash) || (pEntry->key_len != len)) {
		/* Compare unmasked hash and key length. */
		return 0;
	}
	
	if (memcmp(key, pEntry->key, len) != 0) { 
		/* Compare keys. */
		return 0;
	}
	else {
		/* Matched. */
		return 1;
	}
	return 0;
}

/* Return pointer to value in table or NULL if not in table. */
void * hashtable_get(HashTable *table, const char *key, uint32_t len) {
	Entry *pEntry;
	void *result = NULL;
	uint64_t hash = HASH(key, len);

	/* TODO: Check if table is init'd. */
	//printf("Get: key %s hashed to %lu.\n", key, hash);
	pEntry = table->entries[hash & table->mask];
	printf("Get: chain head @ %p.\n", pEntry);
	while (!result && pEntry) {
		if (hashtable_entry_test(pEntry, hash, key, len)) {
			result = pEntry->value;
		}
		pEntry = pEntry->pNext;
	}
	return result;
}

/* Find and delete a value in table. Returns pointer to value or NULL. */
void * hashtable_unset(HashTable *table, const char *key, uint32_t len) {
	Entry *pEntry;
	void *result = NULL;
	uint64_t hash = HASH(key, len);

	/* TODO: Check if table is init'd. */
	pEntry = table->entries[hash & table->mask];
	/* Search chain for entry. */
	for (;!hashtable_entry_test(pEntry, hash, key, len) && pEntry; pEntry = pEntry->pNext);
	/* If pEntry is not NULL, entry has been found */
	if (pEntry) {
		printf("Unset: found item @ %p.\n", pEntry);
		if (!pEntry->pPrev) {
			/* Entry is the head. */
			table->entries[hash & table->mask] = pEntry->pNext;
		}
		else {
			/* Set next of prev element. */
			pEntry->pPrev->pNext = pEntry->pNext;
		}
		if (pEntry->pNext) {
			/* Entry is not the tail - set prev of next element. */
			pEntry->pNext->pPrev = pEntry->pPrev;
		}
		/* Free memory. */
		result = pEntry->value;
		free(pEntry->key);
		free(pEntry);
		table->count--;
	}
	return result;
}

/* Insert or update existing value. */
void hashtable_set(HashTable *table, const char *key, uint32_t len, void *value) {
	Entry *pEntry;
	/* 0 = Chain is empty, 1 = Found, 2 = Hit tail without finding. */
	int action = 0; 
	uint64_t hash = HASH(key, len);

	//printf("Insert: key %s hashed to %lu.\n", key, hash);
	/* TODO: Check if table is init'd. */
	pEntry = table->entries[hash & table->mask];
	/* Search chain. */
	while (!action && pEntry) {
		if (hashtable_entry_test(pEntry, hash, key, len)) {
			action = 1;
		}
		else if (!pEntry->pNext) {
			/* The current item is the tail. */
			action = 2;
		}
		else {
			pEntry = pEntry->pNext;
		}
	}

	if (action != 1) {
		if (action == 0) {
			/* Empty chain insert. */
			puts("Set: inserting value (head).");
			pEntry = malloc(sizeof(Entry));
			table->entries[hash & table->mask] = pEntry;
			pEntry->pPrev = NULL;
		}
		else {
			/* Non-empty chain insert. */
			puts("Set: inserting value.");
			pEntry->pNext = malloc(sizeof(Entry));
			/* Point new entry back at tail. */
			pEntry->pNext->pPrev = pEntry;
			pEntry = pEntry->pNext;
		}
		pEntry->pNext = NULL;
		pEntry->hash = hash;
		pEntry->key = malloc(len * sizeof(char));
		pEntry->key_len = len;
		memcpy(pEntry->key, key, len * sizeof(char));
		printf("Set: Allocated %u bytes for key %s.\n", len, key);
		table->count++;
	}
	else {
		puts("Set: updating value.");
	}
	/* Set value. */
	pEntry->value = value;
	if (table->count > table->size) {
		hashtable_resize(table);
	}
}

/* Iterator interface follows... */

/* Set the internal pointers to the first element in the table. */
void hashtable_iter_first(HashTable *table) {
	unsigned int pos = 0;

	if (table->count) {
		/* We know there's an element, iterate until we find it. (would out of bounds if empty) */
		for (;(pos < table->size) && !table->entries[pos]; pos++)
			;
		/* Set pointers. */
		table->pEntry = table->entries[pos];
		table->entries_pos = pos;
	}
	else {
		table->pEntry = NULL;
	}
}

/* Set the internal pointers to the next element in the table. 0 if end of table. */
int hashtable_iter_next(HashTable *table) {
	uint32_t pos = table->entries_pos + 1;

	if (!table->pEntry) {
		return 0;
	}

	if (table->pEntry->pNext) {
		table->pEntry = table->pEntry->pNext;
		return 1;
	}

	/* Look for the next element. */
	for (;(pos < table->size) && !table->entries[pos]; pos++)
		;
	if (pos < table->size) {
		/* Element found */
		table->pEntry = table->entries[pos];
		table->entries_pos = pos;
		return 1;
	}

	table->pEntry = NULL;
	return 0;
}

/* Consider making these macros. */
char * hashtable_iter_key(HashTable *table) {
	return (table == NULL || table->pEntry == NULL) ? (char *)NULL : table->pEntry->key;
}

unsigned int hashtable_iter_key_len(HashTable *table) {
	return (table == NULL || table->pEntry == NULL) ? 0 : table->pEntry->key_len;
}

void * hashtable_iter_value(HashTable *table) {
	return (table == NULL || table->pEntry == NULL) ? NULL : table->pEntry->value;
}
