#include <stdio.h>
#include <hashtable.h>
#include <string.h>

HashTable test;

char key1[] = "EzEzEzEzEzEzEzEz";
char key2[] = "EzEzEzEzEzEzEzFY";
char data1[] = "my test string";
char data2[] = "another test string";
char data3[] = "Overwrite a string somewhere.";

char * keys[] = {"just", "a", "few", "keys", "for", "testing", "and", "another", "some", "more", "what's the story duplicate?", "test?"};
char * values[] = {"value1", "value2", "value3", "value4", "value5", "value6", "value7", "value8", "value9", "value10", "value11", "value12"};

#define TRY_GET(K) 	res = hashtable_get(&test, K, sizeof(K) - 1); if (res != NULL) printf("%s => %s\n", (K), res); else printf("%s is not in table.\n", (K));

int main(int argv, char * argc[]) {
	char * res;
	uint32_t i;

	hashtable_init(&test, 7);

	for (i = 0; i < (sizeof(keys) / sizeof(char *)); i++) {
		printf("Set %s => %s\n", keys[i], values[i]);
		hashtable_set(&test, keys[i], strlen(keys[i]) + 1, values[i]);
	}

	//hashtable_debug(&test);

/*
	printf("Inserting %s into table.\n", key1);
	hashtable_set(&test, key1, sizeof(key1) - 1, data1);

	TRY_GET(key1);

	printf("Inserting %s into table.\n", key2);
	hashtable_set(&test, key2, sizeof(key2) - 1 , data2);

	TRY_GET(key1);
	TRY_GET(key2);
	puts("Table state:");
	hashtable_debug(&test);

	hashtable_set(&test, key1, sizeof(key1) - 1, data3);

	printf("Hash table contains %i items.\n", test.count);
	*/
	hashtable_debug(&test);

	hashtable_iter_first(&test);
	do  {
		printf("[%s] => %s\n", hashtable_iter_key(&test), hashtable_iter_value(&test));
	} while (hashtable_iter_next(&test));
	
	hashtable_deinit(&test);
	return 0;
}
