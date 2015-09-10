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

void hashtable_init(HashTable * table, unsigned int size) {
	/* Find power of two greater than size. Min size is 8 elems. */
	unsigned int i = 3;
	for (;(1U << i) < size; i++);
	table->num_elements = 0;
	table->size = 1 << i;
	table->mask = table->size - 1;
	printf("Init: HashTable with size %u (requested %u).\n", table->size, size);
	table->entries = calloc(table->size, sizeof(Entry));
}

void hashtable_deinit(HashTable * table) {
	/* Need to iterate over and free everything */
	free(table->entries);
}

void hashtable_resize(HashTable * table) {

}

void hashtable_debug(HashTable * table) {
	unsigned int i;
	Item * pItem;
	for (i = 0; i < table->size; i++) {
		pItem = table->entries[i].pHead;
		printf("%i => ", i);
		while (pItem != NULL) {		
			printf("%p, ", pItem);
			pItem = pItem->pNext;
		} 
		puts(" NULL.");
	}
}

/* Compare an entry, with full hash, key and length. */
static int inline hashtable_entry_test(Item * i, unsigned long hash, const char * key, unsigned int len) {
	/* Always false for null items. */
	if (i == NULL) {
		return 0;
	}
	/* Compare unmasked hash and key length. */
	if ((i->hash != hash) || (i->key_len != len)) {
		return 0;
	}
	/* Compare keys */
	if (memcmp(key, i->key, len) != 0) { 
		return 0;
	}
	return 1;
}

/* Return pointer to value in table or null if not in table. */
void * hashtable_get(HashTable * table, const char * key, unsigned int len) {
	Item * pItem;
	unsigned long hash = HASH(key, len);
	/* TODO: Check if table is init'd. */
	printf("Get: key %s hashed to %lu.\n", key, hash);
	pItem = table->entries[hash & table->mask].pHead;
	/* Is this a list? */
	printf("Get: chain head @ %p.\n", pItem);
	while (pItem != NULL) {
		if (hashtable_entry_test(pItem, hash, key, len)) {
			return pItem->value;
		}
		pItem = pItem->pNext;
	}
	return NULL;
}

/* Find and delete a value in table. Returns pointer to value or NULL */
void * hashtable_unset(HashTable * table, const char * key, unsigned int len) {
	Entry * pEntry;
	Item * pItem;
	void * result = NULL;
	unsigned long hash = HASH(key, len);
	/* TODO: Check if table is init'd. */
	pEntry = &(table->entries[hash & table->mask]);
	pItem = pEntry->pHead;
	while (pItem != NULL) {
		if (hashtable_entry_test(pItem, hash, key, len)) {
			break;
		}
		pItem = pItem->pNext;
	}
	if (pItem != NULL) {
		printf("Delete: found item @ %p.\n", pItem);
		if (pItem->pPrev == NULL) {
			/* Item is the head. */
			pEntry->pHead = pItem->pNext;
		}
		else {
			pItem->pPrev->pNext = pItem->pNext;
		}
		if (pItem->pNext == NULL) {
			/* Item is the tail. */
			pEntry->pTail = pItem->pPrev;
		}
		else {
			pItem->pNext->pPrev = pItem->pPrev;
		}
		/* Free memory. */
		result = pItem->value;
		free(pItem->key);
		free(pItem);
		table->num_elements--;
	}
	return  result;
}

/* Insert or update existing value. */
void hashtable_set(HashTable * table, const char * key, unsigned int len, void * value) {
	Entry * pEntry;
	Item * pItem;
	unsigned long hash = HASH(key, len);
	printf("Insert: key %s hashed to %lu.\n", key, hash);
	/* TODO: Check if table is init'd. */
	/* TODO: Resize if needed. */
	pEntry = &(table->entries[hash & table->mask]);
	pItem = pEntry->pHead;

	while (pItem != NULL) {
		if (hashtable_entry_test(pItem, hash, key, len)) {
			break;
		}		
		pItem = pItem->pNext;
	}

	if (pItem == NULL) {
		puts("Set: inserting value.");
		pItem = malloc(sizeof(Item));
		if (pEntry->pTail != NULL) {
			/* Make tail item's next point to new item. */
			pEntry->pTail->pNext = pItem;
		}
		/* Make this item point to list tail (whether NULL or not). */
		pItem->pPrev = pEntry->pTail;
		/* This item is the new tail */
		pEntry->pTail = pItem;
		/* This item points to nothing. */
		pItem->pNext = NULL;
		if (pEntry->pHead == NULL) {
			/* This is the first in the list. */
			pEntry->pHead = pItem;
			pItem->pPrev = NULL;
		}
		pItem->hash = hash;
		pItem->key = malloc(len * sizeof(char));
		pItem->key_len = len;
		memcpy(pItem->key, key, len * sizeof(char));
		table->num_elements++;
	}
	else {
		puts("Set: updating value.");
	}
	pItem->value = value;
}

/* Iterator support */

/* Set the internal pointers to the first element in the table. */
void hashtable_iter_first(HashTable * table) {
	unsigned int pos;
	if (table->num_elements) {
		/* We know there's an element, iterate until we find it. */
		for (pos = 0; (pos < table->num_elements) && (table->entries[pos].pHead != NULL); pos++);
		/* Set pointers. */
		table->pEntry = &(table->entries[pos]);
		table->pItem = table->pEntry->pHead;
	}
}

/* Set the internal pointers to the next element in the table. */
void hashtable_iter_next(HashTable * table) {
	if (table->pEntry && table->pItem) {
		if (table->pItem->pNext) {
			/* Simply move to next. */
		}
		else {
			/* Search for next occupied entry. */
		}
	}
}

/* Consider making these macros. */
char * hashtable_iter_key(HashTable * table) {
	return (table == NULL || table->pItem == NULL) ? (char *)NULL : table->pItem->key;
}

unsigned int hashtable_iter_key_len(HashTable * table) {
	return (table == NULL || table->pItem == NULL) ? 0 : table->pItem->key_len;
}

void * hashtable_iter_value(HashTable * table) {
	return (table == NULL || table->pItem == NULL) ? NULL : table->pItem->value;
}
