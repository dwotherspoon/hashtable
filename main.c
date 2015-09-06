#include <stdio.h>
#include <hashtable.h>

HashTable test;


char key1[] = "EzEzEzEzEzEzEzEz";
char key2[] = "EzEzEzEzEzEzEzFY";
char data1[] = "my test string";
char data2[] = "another test string";
char data3[] = "Overwrite a string somewhere.";

#define TRY_GET(K) 	res = hashtable_get(&test, K, sizeof(K) - 1); if (res != NULL) printf("%s => %s\n", K, res); else printf("%s is not in table.\n", K);

int main(int argv, char * argc[]) {
	char * res;
	hashtable_init(&test, 7);
	res = hashtable_get(&test, key1, sizeof(key1) - 1);
	TRY_GET(key1);

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

	TRY_GET(key1);
	TRY_GET(key2);
	puts("Table state:");
	hashtable_debug(&test);

	hashtable_deinit(&test);
	return 0;
}
