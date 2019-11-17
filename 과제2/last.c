#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

FILE *opennow;
char isright[40];
char islast[40];

int main() {
	char present[256];
	for(int i = 0; i < 40; i++) 
		islast[i] = 'a';
	while(1){
		opennow = fopen("/test/.last", "r");
		while(fgets(present, 256, opennow) != NULL) {
			memset(isright, 0, sizeof(isright));
			sscanf(present, "%s", isright);
			if(strcmp(isright, islast) != 0){
				strcpy(islast, isright);
				FILE *openafter = fopen("/test/changes.txt", "a");
				fputs(isright, openafter);
				fputs("\n", openafter);
			}
		}
		sleep(1);
		fclose(opennow);
	}
	return 0;
}
