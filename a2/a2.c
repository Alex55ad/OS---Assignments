#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include "a2_helper.h"

#define MAX_NR_OF_THREADS 50
pthread_mutex_t mutex;
pthread_cond_t cond1,cond2;


	void *P4_thread_function(void *thread_id)
{
    int identifier = *(int *)thread_id;
    if (identifier == 1) // thread T4.1
    {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond1,&mutex);
        info(BEGIN, 4, identifier);
        info(END, 4, identifier);
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&cond2);
    }
    else if (identifier == 4) //Thread T4.4
    {
        info(BEGIN, 4, identifier);
        pthread_cond_broadcast(&cond1);
        pthread_cond_wait(&cond2,&mutex);
        info(END, 4, identifier);

    }
    else
    {
        info(BEGIN, 4, identifier);
        info(END, 4, identifier);
    }
    return NULL;
}

void *P5_thread_function(void *thread_id)
{
    int identifier = *(int *)thread_id;
    info(BEGIN, 5, identifier);
    info(END, 5, identifier);
    return NULL;
}

void create_threads(int pid, int nr_of_threads)
{
    pthread_t thread_array[MAX_NR_OF_THREADS];
    int thread_identifiers[MAX_NR_OF_THREADS];
    pthread_mutex_init(&mutex, NULL);   // initializing the mutex with NULL
    pthread_cond_init(&cond1, NULL);     // initializing the mutex's condition variable with NULL
    pthread_cond_init(&cond2, NULL);     // initializing the mutex's condition variable with NULL

    for (int i = 1; i <= nr_of_threads; i++)
    {
        thread_identifiers[i] = i; // assign the thread an identifier: 1,2, ..., N
        if (pid == 4)
        {
            pthread_create(&thread_array[i], NULL, P4_thread_function, (int *)&thread_identifiers[i]);
        }
        if (pid == 5)
        {
            pthread_create(&thread_array[i], NULL, P5_thread_function, (int *)&thread_identifiers[i]);
        }
    }
    
    for (int i = 1; i <= nr_of_threads; i++)
    {
        pthread_join(thread_array[i], NULL); // join all created threads;
    }
    
    pthread_mutex_destroy(&mutex);   // destroying the mutex
    pthread_cond_destroy(&cond1);     // destroying the mutex's condition variable
    pthread_cond_destroy(&cond2);     // destroying the mutex's condition variable
}

int main()
{
    init();
    // pid_t pid;
    info(BEGIN, 1, 0);
    if (fork() == 0)
    {
        info(BEGIN, 2, 0);

        if (fork() == 0)
        {
            info(BEGIN, 3, 0);

            if (fork() == 0)
            {
                info(BEGIN, 6, 0);

                info(END, 6, 0);
                exit(0); // Exit P6
            }
            wait(NULL); // waiting for P6 to finish
            info(END, 3, 0);
            exit(0); // Exit P3
        }
        if (fork() == 0)
        {
            info(BEGIN, 5, 0);
            create_threads(5, 49);
            info(END, 5, 0);
            exit(0); // EXIT P5
        }
        wait(NULL); // wait for P3 to finish
        wait(NULL); // wait for P5 to finish
        info(END, 2, 0);
        exit(0); // exit P2
    }
    if (fork() == 0)
    {
        info(BEGIN, 4, 0);

        if (fork() == 0)
        {
            info(BEGIN, 7, 0);

            info(END, 7, 0);
            exit(0); // exit P7
        }
        if (fork() == 0)
        {
            info(BEGIN, 8, 0);

            info(END, 8, 0);
            exit(0); // exit P8
        }
        wait(NULL); // wait for P7 to finish
        wait(NULL); // wait for P8 to finish
        create_threads(4, 4);
        info(END, 4, 0);
        exit(0); // exit P4
    }
    wait(NULL); // wait for P2 to finish
    wait(NULL); // wait for P4 to finish
    info(END, 1, 0);
    return 0;
}