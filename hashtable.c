#include <hashtable.h>
#include <stdlib.h>
#include <string.h>;
/* (Mostly PHP's unrolled implementation.)
 * DJBX33A (Daniel J. Bernstein, Times 33 with Addition)
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

#define DJBX_ROUND(h, k) hash = ((h << 5) + h) + *k++;

static unsigned long inline hash_djbx(const char * key, unsigned int len) {
	register unsigned long hash = 5381;
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
	/* Do remaining rounds. (n.b. fall through) */
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

void hashtable_init(HashTable * table, const unsigned int size) {
	/* Find power of two greater than size. Min size is 8 elems. */
	unsigned int i = 3;
	for (;(1U << i) < size; i++);
	table->num_elements = 0;
	table->size = 1 << i;
	table->mask = table->size - 1;
	printf("Init: HashTable with size %u (requested %u).\n", table->size, size);
	table->entries = malloc(table->size * sizeof(Entry *));
}

void hashtable_deinit(HashTable * table) {
	free(table->entries);
}

void hashtable_resize(HashTable * table) {

}

void hashtable_debug(HashTable * table) {
	unsigned int i;
	Entry * pEntry;
	for (i = 0; i < table->size; i++) {
		pEntry = table->entries[i];
		printf("%i => ", i);
		while (pEntry != NULL) {		
			printf("%p, ",pEntry);
			pEntry = pEntry->pNext;
		} 
		puts(" NULL.");
	}
}

/* Compare an entry, with full hash, key and length. */
static int inline hashtable_entry_test(Entry * e, const unsigned long hash, const char * key, unsigned int len) {
	/* Compare unmasked hash and key length. */
	if ((e->hash != hash) || (e->key_len != len)) {
		return 0;
	}
	/* Compare keys */
	if (strncmp(key, e->key, len) != 0) { 
		return 0;
	}
	return 1;
}

void * hashtable_get(HashTable * table, const char * key, unsigned int len) {
	Entry * pEntry;
	unsigned long hash = HASH(key, len);
	/* TODO: Check if table is init'd. */
	/* Hash the key. */
	printf("Get: key %s hashed to %lu.\n", key, hash);
	pEntry = table->entries[hash & table->mask];
	/* Is this a list? */
	printf("Get: chain head @ %p.\n", pEntry);
	while (pEntry != NULL) {
		if (hashtable_entry_test(pEntry, hash, key, len)) {
			return pEntry->value;
		}
		pEntry = pEntry->pNext;
	}
	return NULL;
}

void hashtable_set(HashTable * table, const char * key, void * data) {

}

/* Insert an entry given that we're sure there isn't one. */
void hashtable_insert(HashTable * table, const char * key, unsigned int len, void * value) {
	Entry * pEntry;
	unsigned long hash = HASH(key, len);
	printf("Insert: key %s hashed to %lu.\n", key, hash);
	/* TODO: Check if table is init'd. */
	/* TODO: Resize if needed */
	pEntry = table->entries[hash & table->mask];
	if (pEntry != NULL) {
		/* Insert by chaining */
		puts("Insert: collision! Using chaining.");
		while (pEntry->pNext != NULL) {
			pEntry = pEntry->pNext;
		}
		pEntry->pNext = malloc(sizeof(Entry));
		pEntry->pNext->pPrev = pEntry;
		pEntry = pEntry->pNext;
	}
	else {
		pEntry = malloc(sizeof(Entry));
		table->entries[hash & table->mask] = pEntry;
		pEntry = table->entries[hash & table->mask];
	}
	pEntry->hash = hash;
	pEntry->value = value;
	pEntry->key = malloc(len * sizeof(char));
	pEntry->key_len = len;
	memcpy(pEntry->key, key, len * sizeof(char));
}
