#include "stdio.h"
#include "pthread.h"
#include "semaphore.h"
//g++ -std=c++11 -pthread msort.cpp -o msort
sem_t border_sem;
sem_t cs_sem;
int num_of_threads, num_of_elems;
timespec time_start, time_end;
long unsigned convert_time() {
    return (time_end.tv_sec * 1000 + time_end.tv_nsec / 1000000) - (time_start.tv_sec * 1000 + time_start.tv_nsec / 1000000);
}
struct sorting_blocks
{
    int* start;
    int len;
    sorting_blocks* next;
    sem_t cur_sema;
    sem_t bord;
    sorting_blocks* daughter1;
    sorting_blocks* daughter2;
    int num;
};

int* start1;
volatile sorting_blocks* start_of_line;
volatile sorting_blocks* end_of_line;
volatile sorting_blocks* cur_elem;
FILE* in;
sorting_blocks* addition(int* start, int len, sorting_blocks* daugter1, sorting_blocks* daughter2)
{
    sorting_blocks* elem = new sorting_blocks;
    elem->start = start;
    elem->len = len;
    elem->next = NULL;
    elem->num = 0;
    sem_init(&elem->cur_sema, 0, 1);
    if (len <= 1000)
    {
        elem->daughter1 = NULL;
        elem->daughter2 = NULL;
    }
    else
    {



        elem->daughter1 = daugter1;
        elem->daughter2 = daughter2;
    }
    if (start_of_line == NULL)
    {
        start_of_line = elem;
        cur_elem = start_of_line;
        end_of_line = elem;
        return elem;
    }
    end_of_line->next = elem;
    end_of_line = elem;
    return elem;
}

sorting_blocks* get_cur_elem()
{
    sorting_blocks* elem = (sorting_blocks*)cur_elem;
    cur_elem = cur_elem->next;
    return elem;
}

void one_merge(int* start1, int len1, int* start2, int len2)
{
    int* temp1 = new int[len1];
    int* temp2 = new int[len2];
    for (int i = 0; i < len1; i++)
    {
        temp1[i] = start1[i];
    }
    for (int i = 0; i < len2; i++)
    {
        temp2[i] = start2[i];
    }
    int a = 0;
    int b = 0;
    while (a < len1 || b < len2)
    {
        if ((a < len1) && !(b < len2))
        {
            start1[a + b] = temp1[a];
            a++;
            continue;
        }
        if (!(a < len1) && (b < len2))
        {
            start1[a + b] = temp2[b];
            b++;
            continue;
        }
        if (temp1[a] >= temp2[b])
        {
            start1[a + b] = temp2[b];
            b++;
            continue;
        }
        if ((temp1[a] < temp2[b]))
        {
            start1[a + b] = temp1[a];
            a++;
            continue;
        }
    }
}

void merdging(int* start, int len)
{
    if (len <= 1) { return; }
    else
    {
        merdging(start, len / 2);
        merdging(start + len / 2, len - len / 2);
        one_merge(start, len / 2, start + len / 2, len - len / 2);
    }
}


void* thread_code(void* param)
{
    do
    {
        sem_wait(&border_sem);
        sem_wait(&cs_sem);
        if (cur_elem == NULL)
        {
            sem_post(&cs_sem);
            sem_post(&border_sem);
            break;
        }
        sorting_blocks* elem = get_cur_elem();
        sem_post(&cs_sem);
        sem_wait(&elem->cur_sema);
        if (elem->len >= 1000)
        {
            sem_wait(&elem->daughter1->cur_sema);
            sem_post(&elem->daughter1->cur_sema);
            sem_wait(&elem->daughter2->cur_sema);
            sem_post(&elem->daughter2->cur_sema);
            one_merge(elem->daughter1->start, elem->daughter1->len, elem->daughter2->start, elem->daughter2->len);
        }
        /*
       else if (elem->len==1){
           sem_post(&elem->cur_sema);
           break;
       }
*/
        else {
            merdging(elem->start, elem->len);
        }
        sem_post(&elem->cur_sema);
        sem_post(&border_sem);
    } while (true);
}

sorting_blocks* make_array_line(int* start, int len)
{
    if (len < 1000) { return addition(start, len, NULL, NULL); }
    else
    {
        sorting_blocks* half2 = make_array_line(start + len / 2, len - len / 2);
        sorting_blocks* half1 = make_array_line(start, len / 2);
        return addition(start, len, half1, half2);
    }
}
void deinit() {
    sem_destroy(&border_sem);
    sem_destroy(&cs_sem);
    printf("num elems %d\n", num_of_elems);
    FILE* out = fopen("output.txt", "a+");
    fprintf(out, "%d\n%d\n", num_of_threads, num_of_elems);
    for (int i = 0; i < num_of_elems; i++) { fprintf(out, "%d ", start1[i]); }
    fclose(out);
    FILE* tim = fopen("time.txt", "a+");
    fprintf(tim, "%ld", convert_time());
    fclose(tim);
    fclose(in);
}
int main()
{
    start_of_line = NULL;
    end_of_line = NULL;
    cur_elem = NULL;
    in = fopen("input.txt", "a+");
    fscanf(in, "%d", &num_of_threads);
    fscanf(in, "%d", &num_of_elems);
    printf("num elems %d\n", num_of_elems);
    int* sort_arr = new int[num_of_elems];
    for (int i = 0; i < num_of_elems; i++) { fscanf(in, "%d", &sort_arr[i]); }
    start1 = sort_arr;
    sem_init(&border_sem, 0, num_of_elems);
    sem_init(&cs_sem, 0, 1);
    make_array_line(start1, num_of_elems);
    clock_gettime(CLOCK_REALTIME, &time_start);
    //if (check(num_of_elems)==1) retur
    printf("start\n");
    pthread_t* arr_of_threads = new pthread_t[num_of_threads];
    for (int i = 0; i < num_of_threads; i++)
    {
        if (pthread_create(&arr_of_threads[i], 0, thread_code, 0) != 0)
        {
            printf("Thread create error\n");
            return -1;
        }
        printf("ok_thread\n");
    }
    for (int i = 0; i < num_of_threads; i++) { pthread_join(arr_of_threads[i], 0); }
    clock_gettime(CLOCK_REALTIME, &time_end);
    deinit();
}
