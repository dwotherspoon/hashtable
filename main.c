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
	char buf[128]; 
	char * cursor;
	FILE * pFile;
	uint32_t i = 0;

	hashtable_init(&test, 7);

	pFile = fopen("/usr/share/dict/words", "r");

	while (fgets(buf, 128, pFile)) {
		/* Remove new line chars */
		for (cursor = buf; *cursor; cursor++) {
			switch (*cursor) {
				case '\n':
				case '\r':
					*cursor = '\0';
					break;
			}
		}

		hashtable_set(&test, buf, strlen(buf) + 1, (void *)i++);	
	}

	

	hashtable_iter_first(&test);
	do  {
		//printf("[%s] => %d\n", hashtable_iter_key(&test), (uint32_t)hashtable_iter_value(&test));
	} while (hashtable_iter_next(&test));

	hashtable_debug(&test);


	i = 0;
	fseek(pFile, 0, SEEK_SET);

	/* Check */
	while(fgets(buf, 128, pFile)) {
		/* Remove new line chars */
		for (cursor = buf; *cursor; cursor++) {
			switch (*cursor) {
				case '\n':
				case '\r':
					*cursor = '\0';
					break;
			}
		}
		putchar(hashtable_get(&test, buf, strlen(buf) + 1) == (void *)i++ ?  '.' : 'X');
	}
	putchar('\n');

	fclose(pFile);
	
	hashtable_free(&test, NULL);
	return 0;
}
