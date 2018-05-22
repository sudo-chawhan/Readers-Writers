#include <semaphore.h>
#include <thread>
#include <vector>
#include <iostream>
#include <atomic>
#include <stdio.h>
#include <ctime>
#include <unistd.h>
#include <time.h>
#include <chrono>
#include <pthread.h>
#include <fstream>

unsigned int randCSTime,randRemTime,kw,kr;
using namespace std;
void *writer(void *i);     // writer thread
void *reader(void *i);      // reader threads

double avg_time=0.0;

sem_t out_mutex;            // semaphore for out_count
sem_t in_mutex;            // semaphore for in_count
sem_t wrt_mutex;          // semaphore for access to writer
sem_t avgmutex;         // semaphore for shared variable avg_time safety
int in_count=0;       // number of readers inside cs
int out_count=0;    // number of readers outside cs
bool wait=false;    // if writer is waiting for entry or not

FILE *ouput_log = fopen("RWFair-log.txt", "w");  //open a file to wirte to it
FILE *output_avg = fopen("Average_time.txt", "a");  //open a file to append to it

int main()
{
    // initialising semaphores
    sem_init(&out_mutex,0,1);   
    sem_init(&in_mutex,0,1);
    sem_init(&wrt_mutex,0,0);
    sem_init(&avgmutex,0,1);

    int nw,nr,csSeed,remSeed;
    ifstream inFile ;
   inFile.open("input.txt");    //we read from the input file
   inFile>>nr ;
   inFile>>nw;
   inFile>>kr;
   inFile>>kw;
   inFile>>csSeed;
   inFile>>remSeed;
    // scanf("%d %d %d %d %d %d",&nw,&nr,&kw,&kr,&csSeed,&remSeed);

    // generating random cs and remainder time
    srand(csSeed);
    randCSTime=rand()%100;
    srand(remSeed);
    randRemTime=rand()%100;

// create 'nw' writer threads
// create 'nr' reader threads

    // creating n pthreads
    pthread_t w[nw],r[nr];
    pthread_attr_t w_attr[nw],r_attr[nr];

    for (int i = 0; i < nw; ++i) {
        pthread_attr_init(&w_attr[i]);
                pthread_create(&w[i],&w_attr[i],writer,(void*)(intptr_t)(i+1));
    }
    for (int i = 0; i < nr; ++i) {
        pthread_attr_init(&r_attr[i]);
                pthread_create(&r[i],&r_attr[i],reader,(void*)(intptr_t)(i+1));
    }
    // joining the threads
    for(int i=0;i<nw;i++)
                pthread_join(w[i],NULL);
    for(int i=0;i<nr;i++)
                pthread_join(r[i],NULL);

    // destroying semaphores
    sem_destroy(&in_mutex);
    sem_destroy(&out_mutex);
    sem_destroy(&wrt_mutex);
    sem_destroy(&avgmutex);

    fprintf(output_avg,"Avg time taken for RW_fair is %f milliseconds\n",avg_time/((nw+nr)*1000));

    return 0;

}

void *writer(void * param){

    int id = (intptr_t)param;

    for(int i=1; i<=kw ;i++){

        // calculating request time
        auto reqTime = std::chrono::system_clock::now();      
        time_t my_time;
        time (&my_time);
        struct tm *timeinfo = localtime (&my_time);
        fprintf(ouput_log,"%d th CS request by Writer Thread %d at %02d:%02d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);

        sem_wait(&in_mutex); // wait in
        sem_wait(&out_mutex); // wait out
        if(in_count==out_count) // if cs is not used by any reader
            sem_post(&out_mutex);   // signal out_mutex
        else{                       // else 
            wait=true;              // current writer is waiting
            sem_post(&out_mutex);    // signal out__mutex
            sem_wait(&wrt_mutex);    // signal wait_mutex
            wait=false;             // current waiter is not waiting anymore
        }


        // calculating entery time
        auto enterTime = std::chrono::system_clock::now();
        time (&my_time);
        timeinfo = localtime (&my_time);
        fprintf(ouput_log,"%d th CS Entry by Writer Thread %d at %02d:%02d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);

        usleep(randCSTime*10);// simulate a thread executing in CS
        
        fprintf(ouput_log,"%d th CS Exit by Writer Thread %d at %02d:%02d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);

        sem_post(&in_mutex);   // signal in_mutex

        // calculating exit time
        time (&my_time);
        timeinfo = localtime (&my_time);

        usleep(randRemTime*10); // simulate a thread executing in Remainder Section

        // adding waiting time to shared variable avg_time
        sem_wait(&avgmutex);
        avg_time += std::chrono::duration_cast<std::chrono::microseconds>(enterTime-reqTime).count();
        sem_post(&avgmutex);    
    } // lagja gale

    pthread_exit(0);
}

void *reader(void * param)
{

    int id = (intptr_t)param;

    for(int i=1; i<=kw ;i++){

        auto reqTime = std::chrono::system_clock::now();
        time_t my_time;
        time (&my_time);
        struct tm *timeinfo = localtime (&my_time);
        fprintf(ouput_log,"%d th CS request by Reader Thread %d at %02d:%02d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);

        sem_wait(&in_mutex);  // wait for in_count access permission
        in_count++;       // increment in count as new reader is entering 
        sem_post(&in_mutex);   // signal mutex

        auto enterTime = std::chrono::system_clock::now();
        time (&my_time);
        timeinfo = localtime (&my_time);
        fprintf(ouput_log,"%d th CS Entry by Reader Thread %d at %02d:%02d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);

        usleep(randCSTime*10);// simulate a thread executing in CS

        sem_wait(&out_mutex);   // wait for out_count access permission
        out_count++;       // increment out_count as we are done reading
        if(wait && in_count==out_count)   // only if writer is waiting and this is the last reader inside cs
            sem_post(&wrt_mutex); // signal wrt_mutex

        fprintf(ouput_log,"%d th CS Exit by Reader Thread %d at %02d:%02d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);

        sem_post(&out_mutex);   // signal out_mutex
        /*
        * Your code for the thread to exit the CS.
        */
        time (&my_time);
        timeinfo = localtime (&my_time);

        usleep(randRemTime*10); // simulate a thread executing in Remainder Section

        sem_wait(&avgmutex);
        avg_time += std::chrono::duration_cast<std::chrono::microseconds>(enterTime-reqTime).count();
        sem_post(&avgmutex);
    }

    pthread_exit(0);
}