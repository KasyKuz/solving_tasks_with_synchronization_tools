
#include "stdio.h"
#include "windows.h"
#include "stdio.h"
#include "iostream"
volatile DWORD TOTAL;
volatile int PHIL;
volatile DWORD start_time;
HANDLE uni_mutex;
HANDLE event;
int N = 5;
enum status
{
    EATING,
    THINKING,
    HUNGRY
};
struct phill_struct
{
    
    status cur_stat;
    

};
struct phill_struct phils[5];
enum status phil_state[5];
DWORD WINAPI thread_entry(void* param)
{
   
    int i = *(int*)param;
    //printf("%d\n", i);
    while (GetTickCount() - start_time <= TOTAL)
    {
        WaitForSingleObject(uni_mutex, INFINITE);
        if ((phil_state[(i + 1) % N] != EATING) && (phil_state[(i +N- 1) % N] != EATING))//.cur_stat
        {
            phil_state[i] = EATING;

            ReleaseMutex(uni_mutex);

            WaitForSingleObject(event, INFINITE);
            std::cout << GetTickCount() - start_time << ":" <<  i + 1  << " T->E\n";
            SetEvent(event);

            Sleep(PHIL);


            phil_state[i] = THINKING;



            WaitForSingleObject(event, INFINITE);
            std::cout << GetTickCount() - start_time << ":" <<  i + 1  << " E->T\n";
            SetEvent(event);
         
            Sleep(PHIL);
            //ReleaseMutex(uni_mutex);
            
          
        }
        else ReleaseMutex(uni_mutex);
       
    }
    return 0;
}
int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("Incorrect num of arguments\n");
        return -1;
    }

    TOTAL=atoi(argv[1]);
    PHIL=atoi(argv[2]);
    
    uni_mutex = CreateMutex(NULL, FALSE, NULL);
    event = CreateEvent(NULL, FALSE, TRUE, NULL);
   
    for (int i = 0; i < 5; i++)
    {
      //  phils[i].cur_stat = THINKING;
        phil_state[i] = THINKING;

    }
    HANDLE threads[5];
    start_time = GetTickCount();
    int phils_nums[5] = { 0,1,2,3,4 };
    for (int i = 0; i < 5; i++)
    {
        threads[i] = CreateThread(0, 0, thread_entry, &phils_nums[i], 0, 0);
    }
    for (int i = 0; i < 5; i++)
    {
        WaitForSingleObject(threads[i], INFINITE);
        CloseHandle(threads[i]);
    }
    CloseHandle(uni_mutex);
    CloseHandle(event);
}
