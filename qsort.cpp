//g++ -std=c++11 -pthread qsort.cpp -o qsort


#include "stdio.h"
#include "pthread.h"
#include "semaphore.h"

using namespace std;

FILE* in;
FILE* output;
FILE* tim;
pthread_mutex_t mutex;
pthread_mutex_t mutex2;
struct sorting_blocks {
    int* start;
    int len;
    int border;
    sorting_blocks* next;
};
int* start1;
volatile sorting_blocks* start_of_line;
volatile sorting_blocks* end_of_line;
timespec start_time, finish_time;
int num_of_threads, num_of_elems;
long unsigned convert_time() {
    return (finish_time.tv_sec * 1000 + finish_time.tv_nsec / 1000000) - (start_time.tv_sec * 1000 + start_time.tv_nsec / 1000000);
}
void addition(int* start, int len)
{
    sorting_blocks* elem = new sorting_blocks;
    elem->start = start;
    elem->len = len;
    elem->next = NULL;
    if (start_of_line == NULL)
    {
        start_of_line = elem;
        end_of_line = elem;
        return;
    }
    end_of_line->next = elem;
    end_of_line = elem;
}

sorting_blocks* pop_elem()
{
    sorting_blocks* ret_elem = (sorting_blocks*)start_of_line;
    start_of_line = start_of_line->next;
    if (start_of_line == NULL)
    {
        end_of_line = NULL;
    }
    return ret_elem;
}

int qsort_(int* start_block, int len)
{
    int med = len / 2;
    int ind_left = 0;
    int ind_right = len - 1;
    while ((ind_left < ind_right))
    {
        while ((start_block[ind_left] <= start_block[med]) && (ind_left < med))
        {
            ind_left++;
        }
        while ((start_block[ind_right] > start_block[med]) && (ind_right > med))
        {
            ind_right--;
        }
        if ((ind_left != med) && (ind_right != med))
        {
            int c = start_block[ind_left];
            start_block[ind_left] = start_block[ind_right];
            start_block[ind_right] = c;
            continue;
        }
        if (ind_left != ind_right)
        {
            if (ind_left == med)
            {
                int c = start_block[ind_left];
                start_block[ind_left] = start_block[ind_right];
                start_block[ind_right] = c;
                med = ind_right;
            }
            else
            {
                int c = start_block[ind_left];
                start_block[ind_left] = start_block[ind_right];
                start_block[ind_right] = c;
                med = ind_left;
            }
        }
    }

    return med;
}

void rec_sort(int* start, int len)
{
    int middle = qsort_(start, len);
    if (len - middle <= 3)
    {
        if (len - middle == 3)
        {
            if (start[len - 1] < start[len - 2])
            {
                int c = start[len - 1];
                start[len - 1] = start[len - 2];
                start[len - 2] = c;
            }
        }
    }
    else
    {
        rec_sort(start + middle + 1, len - middle - 1);
    }
    if (middle <= 2)
    {
        if (middle == 2)
        {
            if (start[1] < start[0])
            {
                int c = start[1];
                start[1] = start[0];
                start[0] = c;
            }
        }
    }
    else
    {
        rec_sort(start, middle);
    }
}


void* thread_func(void* param)
{
    do
    {
        pthread_mutex_lock(&mutex2);
        long old_val;
        if (start_of_line == NULL)
        {
            pthread_mutex_unlock(&mutex2);
            break;
        }
        sorting_blocks* cur_elem = pop_elem();

        pthread_mutex_unlock(&mutex2);
        int* start = cur_elem->start;
        int len = cur_elem->len;
        if (len < 1000) { rec_sort(start, len); }

        /*
     else if (elem->len==1){
         sem_post(&elem->cur_sema);
         break;
     }
*/
        else
        {
            int med = qsort_(start, len);
            pthread_mutex_lock(&mutex2);
            addition(start + med + 1, len - med - 1);
            addition(start, med);
            pthread_mutex_unlock(&mutex2);

        }
    } while (1);
    return 0;
}
void deinit() {
    printf("End of sort\n");
    output = fopen("output.txt", "a+");
    fprintf(output, "%d\n%d\n", num_of_threads, num_of_elems);
    for (int i = 0; i < num_of_elems; i++) { fprintf(output, "%d ", start1[i]); }
    tim = fopen("time.txt", "a+");
    fprintf(tim, "%ld\n", convert_time());
    fclose(output);
    fclose(tim);
    printf("\n");
    fclose(in);
}
int main()
{
    printf("begin\n");
    in = fopen("input.txt", "a+");
    fscanf(in, "%d", &num_of_threads);
    fscanf(in, "%d", &num_of_elems);
    printf("num elems %d\n", num_of_elems);
    int* sort_arr = new int[num_of_elems];
    for (int i = 0; i < num_of_elems; i++) { fscanf(in, "%d", &sort_arr[i]); }
    start1 = sort_arr;
    //printf("mass load\n");
    start_of_line = NULL;
    end_of_line = NULL;
    pthread_t* arr_of_threads = new pthread_t[num_of_threads];
    int threads_count = num_of_threads;
    pthread_mutex_init(&mutex, 0);

    pthread_mutex_init(&mutex2, 0);
    addition(start1, num_of_elems);
    clock_gettime(CLOCK_REALTIME, &start_time);

    for (int i = 0; i < num_of_threads; i++) {
        if (pthread_create(&arr_of_threads[i], 0, thread_func, (void*)(sort_arr)) != 0)
        {
            printf("Thread create error\n");
            return -1;
        }
    }
    for (int i = 0; i < threads_count; i++) { pthread_join(arr_of_threads[i], 0); }
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex2);
    clock_gettime(CLOCK_REALTIME, &finish_time);
    deinit();



}
