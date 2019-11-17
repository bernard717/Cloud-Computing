#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

char input[20];
char str1[20];
char *str2 = ",";
char output[40];

int main() {
	printf("Write\n");
	scanf("%s", input);
	struct timeval whattime;
	gettimeofday(&whattime, NULL);
	
	long long time_in_mill = (whattime.tv_sec) * 1000 + (whattime.tv_usec) / 1000;

	sprintf(str1, "%lld", time_in_mill);

	sprintf(output, "%s%s%s", str1, str2, input);

	FILE *fp = fopen("/test/.last", "w");
	fputs(output, fp);
	fclose(fp);

	return 0;
}