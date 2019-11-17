// gcc 명령어: gcc dvam.c -o dvam.out

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

//this code needs to running vm more than one

FILE *openxentop;	// file pointer
FILE *openxenlist;	// file pointer for "xl list"
char isright[20];
char areright[20];	// for first call of "xl list"
char amright[20];	// for second call of "xl list"

struct var {
	char name[10];  // name
	char state[8];  // state
	int vcpu;		// # of vcpus
	int cpu_sec;  // execution time
	double cpu_per;  // utilization
};//vm info structure

void getxentop();

int main() {

	openxentop = popen("xentop -b d 1", "r"); // command xentop, d 1 means that xentop's info will be updated per 1sec
	
	struct tm *t;			// 시간 표시를 위한 식
	time_t timer;			// 시간 표시를 위한 식

	timer = time(NULL);		// 시간 표시를 위한 식
	t = localtime(&timer);	// 시간 표시를 위한 식

	while (1) {
		printf("[Time] %d-%d-%d %d:%d:%d\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
		getxentop();
		sleep(3);
	}
	pclose(openxentop);
	return 0;
}

void getxentop() {

	char topinfo[256];
	char bottominfo[256];		// xl list를 받아오기 위함
	struct var topvar;	
	int dump1, dump2;			// xl list에서 값을 불러올 때 중간에 필요없는 값은 dump1, dump2에 넣음
	int	nowvcpu;				// pvm1의 vcpu 값을 저장해둠(나중에 pvm2 값과 비교할 때 사용	
	double now;					// pvm1's cpu_per (나중에 pvm2 값과 비교할 때 사용)
	openxenlist = popen("xl list", "r");	// xl list를 실행해서 불러오도록 함
	int count = 2; // assume that two vm are running...
	while (fgets(topinfo, 256, openxentop) != NULL) {			
		memset(isright, 0, sizeof(isright));
		sscanf(topinfo, "%s", isright);
		if (strcmp(isright, "NAME") == 0) continue; // we just need vm's info...
		if (strcmp(isright, "Domain-0") == 0) continue;

		if (count == 2) {						// pvm1일 때, xl list의 3번째 줄에서 vcpu 개수 불러오기
			while (fgets(bottominfo, 256, openxenlist) != NULL) {
				memset(areright, 0, sizeof(areright));
				sscanf(bottominfo, "%s", areright);
				if (strcmp(areright, "NAME") == 0) continue; // we just need vm's info...
				if (strcmp(areright, "Domain-0") == 0) continue;
				sscanf(bottominfo, "%s %d %d %d", topvar.name, &dump1, &dump2, &topvar.vcpu); break;
				}
			}

		if (count == 1) {						// pvm2일 때, xl list의 4번째 줄에서 vcpu 개수 불러오기
			while (fgets(bottominfo, 256, openxenlist) != NULL) {
				memset(amright, 0, sizeof(amright));
				sscanf(bottominfo, "%s", amright);
				if (strcmp(amright, "NAME") == 0) continue; // we just need vm's info...
				if (strcmp(amright, "Domain-0") == 0) continue;
				if (strcmp(amright, "pvm1") == 0) continue;
				sscanf(bottominfo, "%s %d %d %d", topvar.name, &dump1, &dump2, &topvar.vcpu); break;
			}
		}

		sscanf(topinfo, "%s %s %d %lf", topvar.name, topvar.state, &topvar.cpu_sec, &topvar.cpu_per);
		printf("[VM] name: %s, cpu(%): %.1f\n, vcpus: %d", topvar.name, topvar.cpu_per, topvar.vcpu);
		if (count == 2) {																				// pvm2의 vcpu 개수와 cpu_per을 저장해둠(뒤에서 비교)
			now = topvar.cpu_per;
			nowvcpu = topvar.vcpu;
		}
		if (count == 1 && now * 1.2 < topvar.cpu_per && nowvcpu != 1 && now > 1) {						// pvm2의 cpu_per이 더 큰데 pvm1의 vcpu 개수가 아직 2개일 때
			system("xl vcpu-set pvm1 1");																// cpu 사용률이 2배보다 약간 적게 차이 날 때 반영되지 않는 것을 막기 위해 1.2만을 곱해서 비교함  		
			system("xl vcpu-set pvm2 2");
			printf("[DVAM] pvm1 --> 1\n");
			printf("[DVAM] pvm2 --> 2\n");
		}
		else if (count == 1 && now > 1.2 * topvar.cpu_per && topvar.vcpu != 1 && topvar.cpu_per > 1) {	// pvm1의 cpu_per이 더 큰데 pvm2의 vcpu 개수가 아직 2개일 때
			system("xl vcpu_set pvm2 1");																// cpu 사용률이 2배보다 약간 적게 차이 날 때 반영되지 않는 것을 막기 위해 1.2만을 곱해서 비교함
			system("xl vcpu_set pvm1 2");
			printf("[DVAM] pvm1 --> 2\n");
			printf("[DVAM] pvm2 --> 1\n");
		}
		else if (count == 1 && now < 1 && topvar.cpu_per > 1) {											// pvm1의 작업만 끝났을 때, pvm1은 1로, pvm2는 그대로
			system("xl vcpu_set pvm1 1");
			system("xl vcpu_set pvm2 2");
			printf("[DVAM] pvm1 --> 1\n")
		}
		else if (count == 1 && now > 1 && topvar.cpu_per < 1) {											// pvm2의 작업만 끝났을 때, pvm2는 1로, pvm1은 그대로
			system("xl vcpu_set pvm1 2");
			system("xl vcpu_set pvm2 1");
			printf("[DVAM] pvm2 --> 1\n")
		}
		else if (count == 1 && now < 1 && topvar.cpu_per < 1 && (nowvcpu != 2 || topvar.vcpu != 2)) {	// pvm1, pvm2의 작업 모두 끝났을 때, default 값 할당
			system("xl vcpu_set pvm1 2");
			system("xl vcpu_set pvm2 2");
			printf("[DVAM] pvm1 --> 2\n");
			printf("[DVAM] pvm2 --> 2\n");
		}
		count--;
		if (count == 0) break;
	}
	pclose(openxenlist);			
	printf("[VM count] : 2\n")																			// VM count 출력
}


/*
최대 vcpu 개수를 4개라고 했을 경우 코드

if (count == 2) {																				// pvm2의 vcpu 개수와 cpu_per을 저장해둠(뒤에서 비교)
now = topvar.cpu_per;
nowvcpu = topvar.vcpu;
}
if (count == 1 && now * 1.2 < topvar.cpu_per && nowvcpu != 1 && now > 1) {						// pvm2의 cpu_per이 더 큰데 pvm1의 vcpu 개수가 아직 2개일 때, pvm1의 vcpu는 1개로, pvm2의 vcpu는 4개로
system("xl vcpu-set pvm1 1");
system("xl vcpu-set pvm2 4");
printf("[DVAM] pvm1 --> 1\n");
printf("[DVAM] pvm2 --> 4\n");
}
else if (count == 1 && now > 1.2 * topvar.cpu_per && topvar.vcpu != 1 && topvar.cpu_per > 1) {	// pvm1의 cpu_per이 더 큰데 pvm2의 vcpu 개수가 아직 2개일 때, pvm2의 vcpu는 1개로, pvm1의 vcpu는 4개로
system("xl vcpu_set pvm2 1");
system("xl vcpu_set pvm1 4");
printf("[DVAM] pvm1 --> 4\n");
printf("[DVAM] pvm2 --> 1\n");
}
else if (count == 1 && now < 1 && topvar.cpu_per > 1) {											// pvm1의 작업만 끝났을 때, pvm1은 1로, pvm2는 그대로
system("xl vcpu_set pvm1 1");
system("xl vcpu_set pvm2 4");
printf("[DVAM] pvm1 --> 1\n")
}
else if (count == 1 && now > 1 && topvar.cpu_per < 1) {											// pvm2의 작업만 끝났을 때, pvm2는 1로, pvm1은 그대로
system("xl vcpu_set pvm1 4");
system("xl vcpu_set pvm2 1");
printf("[DVAM] pvm2 --> 1\n")
}
else if (count == 1 && now < 1 && topvar.cpu_per < 1 && (nowvcpu != 2 || topvar.vcpu != 2)) {	// pvm1, pvm2의 작업 모두 끝났을 때, default 값 할당
system("xl vcpu_set pvm1 2");
system("xl vcpu_set pvm2 2");
printf("[DVAM] pvm1 --> 2\n");
printf("[DVAM] pvm2 --> 2\n");
}



*/