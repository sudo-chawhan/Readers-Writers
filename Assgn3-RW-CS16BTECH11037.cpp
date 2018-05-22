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

sem_t mutex;            // semaphore for writer entry into cs
sem_t rwmutex;          // semaphore for shared variable read_count safety
sem_t avgmutex;         // semaphore for shared variable avg_time safety
int read_count=0;       // number of readers inside cs

FILE *ouput_log = fopen("RW-log.txt", "w");  //open a file to wirte to it
FILE *output_avg = fopen("Average_time.txt", "a");  //open a file to append to it

int main()
{
    // initialising semaphores
    sem_init(&mutex,0,1);   
    sem_init(&rwmutex,0,1);
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
    sem_destroy(&mutex);
    sem_destroy(&rwmutex);
    sem_destroy(&avgmutex);

    fprintf(output_avg,"Avg time taken for RW is %f milliseconds\n",avg_time/((nw+nr)*1000));

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

        sem_wait(&rwmutex); // wait writer

        // calculating entery time
        auto enterTime = std::chrono::system_clock::now();
        time (&my_time);
        timeinfo = localtime (&my_time);
        fprintf(ouput_log,"%d th CS Entry by Writer Thread %d at %02d:%02d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);

        usleep(randCSTime*10);// simulate a thread executing in CS
        
        fprintf(ouput_log,"%d th CS Exit by Writer Thread %d at %02d:%02d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);

        sem_post(&rwmutex);   // signal writer

        // calculating exit time
        time (&my_time);
        timeinfo = localtime (&my_time);

        usleep(randRemTime*10); // simulate a thread executing in Remainder Section

        // adding waiting time to shared variable avg_time
        sem_wait(&avgmutex);
        avg_time += std::chrono::duration_cast<std::chrono::microseconds>(enterTime-reqTime).count();
        sem_post(&avgmutex);    } // lagja gale

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

        sem_wait(&mutex);  // wait for read_count access permission
        read_count++;       // increment read count as new reader is entering 
        if(read_count==1)   // only if this is the first reader
            sem_wait(&rwmutex); // then wait for cs permission
        sem_post(&mutex);   // signal mutex

        auto enterTime = std::chrono::system_clock::now();
        time (&my_time);
        timeinfo = localtime (&my_time);
        fprintf(ouput_log,"%d th CS Entry by Reader Thread %d at %02d:%02d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);

        usleep(randCSTime*10);// simulate a thread executing in CS

        sem_wait(&mutex);   // wait for read count access permission
        read_count--;       // decrement readcount as we are done reading
        if(read_count==0)   // only if this is the last reader
            sem_post(&rwmutex); // signal rwmutex

        fprintf(ouput_log,"%d th CS Exit by Reader Thread %d at %02d:%02d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);

        sem_post(&mutex);   // signal mutex
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