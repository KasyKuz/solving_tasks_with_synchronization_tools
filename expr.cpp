
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <stdio.h>
#include <windows.h>
#include <malloc.h>
#include <time.h>
using namespace std;
struct expr_borders {
	int border1;
	int border2;
	int sum;
	int* mas;
};
int find_vars(int numbers) {
    int vars = 1;
    for (int i = 0; i < numbers - 1; i++) {
        vars *= 2;
    }
    return vars;
}
HANDLE sema_to_line;

int col_numbers;

CRITICAL_SECTION cs;
volatile int answer;
int col_vars;



int make_base(int itog,int pl, int *in, int multi, int flag, int pl2) {
	//int itog=in[0];
	int cur_ost = pl & 1;
	for (int j = 1; j < col_numbers; j++) {
		if (flag==1 && ((pl2 & 1) == cur_ost)) return itog;
		cur_ost = pl & 1;
		itog +=  multi*(in[j]) * ( 2*cur_ost- 1);
		pl = pl>>1;
		pl2 = pl2 >> 1;
		cur_ost = pl & 1;
	}
	return itog;
}
DWORD WINAPI thread_entry(void* arg) {
	int cur_answer = 0;
	expr_borders* num = static_cast<expr_borders*>(arg);

	int res = 0;// make_base(num->mas[0], num->border1, num->mas, 1, 0, 0);
	//if (res == num->sum)cur_answer++;;
	int border = num->border1;
	int last_res = res;
	while (border < num->border2) {
		last_res = res;
		if (border == num->border1)res = make_base(num->mas[0], border, num->mas, 1, 0, 0);
		else res = make_base(res, border, num->mas, 2, 1, border - 1);
		border += 1;
		if (res == num->sum) cur_answer++;
	}
	EnterCriticalSection(&cs);
	answer += cur_answer;
	LeaveCriticalSection(&cs);
	
	
	return 0;
}

clock_t t;
FILE* f;
void deinit(int col_threads) {
	DeleteCriticalSection(&cs);
	printf("%d\n", answer);
	f = fopen("output.txt", "w");

	fprintf(f, "%d\n%d\n%d", col_threads, col_numbers, answer);

	fclose(f);

	f = fopen("time.txt", "w");

	fprintf(f, "%d", t * 1000 / CLOCKS_PER_SEC);
	fclose(f);

}
int main() {
	int* in_mas;
	int S;
	f = fopen("input.txt", "r");

	int col_threads = 0;

	fscanf(f, "%d %d", &col_threads, &col_numbers);
	
	printf("%d %d\n", col_threads, col_numbers);
	in_mas = (int*)calloc(col_numbers, sizeof(int));
	int i = 0;
	for (i; i < col_numbers; i++) {
		fscanf(f, "%d", &in_mas[i]);
	}
	fscanf(f, "%d", &S);

	fclose(f);

	col_vars = find_vars(col_numbers);
	
	InitializeCriticalSection(&cs);
	answer = 0;

	HANDLE* handles = (HANDLE*)calloc(col_threads, sizeof(HANDLE));
	sema_to_line = CreateSemaphore(0, 1, 1, 0);
	vector<expr_borders> cur_data(col_threads);


	for (i = 0; i < col_threads; i++) {
		expr_borders el = { (col_vars / col_threads) * i ,(col_vars / col_threads) * (i + 1), S,in_mas};
		if (i == col_threads - 1) el.border2 = col_vars;		
		cur_data[i] = el;
		handles[i] = CreateThread(0, 0, thread_entry, &cur_data[i], 0, NULL);
		
	}

	 t = clock();
	WaitForMultipleObjects(col_threads, handles, TRUE, INFINITE);
	CloseHandle(sema_to_line);
	t = clock() - t;

	for (i = 0; i < col_threads; i++) {
		CloseHandle(handles[i]);
	}
	deinit(col_threads);
	
	

	return 0;
}