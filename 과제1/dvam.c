// gcc ��ɾ�: gcc dvam.c -o dvam.out

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
	
	struct tm *t;			// �ð� ǥ�ø� ���� ��
	time_t timer;			// �ð� ǥ�ø� ���� ��

	timer = time(NULL);		// �ð� ǥ�ø� ���� ��
	t = localtime(&timer);	// �ð� ǥ�ø� ���� ��

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
	char bottominfo[256];		// xl list�� �޾ƿ��� ����
	struct var topvar;	
	int dump1, dump2;			// xl list���� ���� �ҷ��� �� �߰��� �ʿ���� ���� dump1, dump2�� ����
	int	nowvcpu;				// pvm1�� vcpu ���� �����ص�(���߿� pvm2 ���� ���� �� ���	
	double now;					// pvm1's cpu_per (���߿� pvm2 ���� ���� �� ���)
	openxenlist = popen("xl list", "r");	// xl list�� �����ؼ� �ҷ������� ��
	int count = 2; // assume that two vm are running...
	while (fgets(topinfo, 256, openxentop) != NULL) {			
		memset(isright, 0, sizeof(isright));
		sscanf(topinfo, "%s", isright);
		if (strcmp(isright, "NAME") == 0) continue; // we just need vm's info...
		if (strcmp(isright, "Domain-0") == 0) continue;

		if (count == 2) {						// pvm1�� ��, xl list�� 3��° �ٿ��� vcpu ���� �ҷ�����
			while (fgets(bottominfo, 256, openxenlist) != NULL) {
				memset(areright, 0, sizeof(areright));
				sscanf(bottominfo, "%s", areright);
				if (strcmp(areright, "NAME") == 0) continue; // we just need vm's info...
				if (strcmp(areright, "Domain-0") == 0) continue;
				sscanf(bottominfo, "%s %d %d %d", topvar.name, &dump1, &dump2, &topvar.vcpu); break;
				}
			}

		if (count == 1) {						// pvm2�� ��, xl list�� 4��° �ٿ��� vcpu ���� �ҷ�����
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
		if (count == 2) {																				// pvm2�� vcpu ������ cpu_per�� �����ص�(�ڿ��� ��)
			now = topvar.cpu_per;
			nowvcpu = topvar.vcpu;
		}
		if (count == 1 && now * 1.2 < topvar.cpu_per && nowvcpu != 1 && now > 1) {						// pvm2�� cpu_per�� �� ū�� pvm1�� vcpu ������ ���� 2���� ��
			system("xl vcpu-set pvm1 1");																// cpu ������ 2�躸�� �ణ ���� ���� �� �� �ݿ����� �ʴ� ���� ���� ���� 1.2���� ���ؼ� ����  		
			system("xl vcpu-set pvm2 2");
			printf("[DVAM] pvm1 --> 1\n");
			printf("[DVAM] pvm2 --> 2\n");
		}
		else if (count == 1 && now > 1.2 * topvar.cpu_per && topvar.vcpu != 1 && topvar.cpu_per > 1) {	// pvm1�� cpu_per�� �� ū�� pvm2�� vcpu ������ ���� 2���� ��
			system("xl vcpu_set pvm2 1");																// cpu ������ 2�躸�� �ణ ���� ���� �� �� �ݿ����� �ʴ� ���� ���� ���� 1.2���� ���ؼ� ����
			system("xl vcpu_set pvm1 2");
			printf("[DVAM] pvm1 --> 2\n");
			printf("[DVAM] pvm2 --> 1\n");
		}
		else if (count == 1 && now < 1 && topvar.cpu_per > 1) {											// pvm1�� �۾��� ������ ��, pvm1�� 1��, pvm2�� �״��
			system("xl vcpu_set pvm1 1");
			system("xl vcpu_set pvm2 2");
			printf("[DVAM] pvm1 --> 1\n")
		}
		else if (count == 1 && now > 1 && topvar.cpu_per < 1) {											// pvm2�� �۾��� ������ ��, pvm2�� 1��, pvm1�� �״��
			system("xl vcpu_set pvm1 2");
			system("xl vcpu_set pvm2 1");
			printf("[DVAM] pvm2 --> 1\n")
		}
		else if (count == 1 && now < 1 && topvar.cpu_per < 1 && (nowvcpu != 2 || topvar.vcpu != 2)) {	// pvm1, pvm2�� �۾� ��� ������ ��, default �� �Ҵ�
			system("xl vcpu_set pvm1 2");
			system("xl vcpu_set pvm2 2");
			printf("[DVAM] pvm1 --> 2\n");
			printf("[DVAM] pvm2 --> 2\n");
		}
		count--;
		if (count == 0) break;
	}
	pclose(openxenlist);			
	printf("[VM count] : 2\n")																			// VM count ���
}


/*
�ִ� vcpu ������ 4����� ���� ��� �ڵ�

if (count == 2) {																				// pvm2�� vcpu ������ cpu_per�� �����ص�(�ڿ��� ��)
now = topvar.cpu_per;
nowvcpu = topvar.vcpu;
}
if (count == 1 && now * 1.2 < topvar.cpu_per && nowvcpu != 1 && now > 1) {						// pvm2�� cpu_per�� �� ū�� pvm1�� vcpu ������ ���� 2���� ��, pvm1�� vcpu�� 1����, pvm2�� vcpu�� 4����
system("xl vcpu-set pvm1 1");
system("xl vcpu-set pvm2 4");
printf("[DVAM] pvm1 --> 1\n");
printf("[DVAM] pvm2 --> 4\n");
}
else if (count == 1 && now > 1.2 * topvar.cpu_per && topvar.vcpu != 1 && topvar.cpu_per > 1) {	// pvm1�� cpu_per�� �� ū�� pvm2�� vcpu ������ ���� 2���� ��, pvm2�� vcpu�� 1����, pvm1�� vcpu�� 4����
system("xl vcpu_set pvm2 1");
system("xl vcpu_set pvm1 4");
printf("[DVAM] pvm1 --> 4\n");
printf("[DVAM] pvm2 --> 1\n");
}
else if (count == 1 && now < 1 && topvar.cpu_per > 1) {											// pvm1�� �۾��� ������ ��, pvm1�� 1��, pvm2�� �״��
system("xl vcpu_set pvm1 1");
system("xl vcpu_set pvm2 4");
printf("[DVAM] pvm1 --> 1\n")
}
else if (count == 1 && now > 1 && topvar.cpu_per < 1) {											// pvm2�� �۾��� ������ ��, pvm2�� 1��, pvm1�� �״��
system("xl vcpu_set pvm1 4");
system("xl vcpu_set pvm2 1");
printf("[DVAM] pvm2 --> 1\n")
}
else if (count == 1 && now < 1 && topvar.cpu_per < 1 && (nowvcpu != 2 || topvar.vcpu != 2)) {	// pvm1, pvm2�� �۾� ��� ������ ��, default �� �Ҵ�
system("xl vcpu_set pvm1 2");
system("xl vcpu_set pvm2 2");
printf("[DVAM] pvm1 --> 2\n");
printf("[DVAM] pvm2 --> 2\n");
}



*/