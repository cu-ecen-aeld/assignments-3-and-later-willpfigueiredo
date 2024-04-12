#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...) printf("threading DEBUG: " msg "\n" , ##__VA_ARGS__)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    thread_func_args->thread_complete_success = false;
    DEBUG_LOG(" Will sleep before obtain  mutex  thread: %lu wait_to_obtain: %d   wait_to_release: %d",pthread_self(), 
    thread_func_args->wait_to_obtain_us, thread_func_args->wait_to_release_us);
    // TODO: wait,
    usleep(thread_func_args->wait_to_obtain_us);
    // obtain mutex, 
    DEBUG_LOG(" Will obtain mutex  thread: %lu wait_to_obtain: %d   wait_to_release: %d",pthread_self(), 
    thread_func_args->wait_to_obtain_us, thread_func_args->wait_to_release_us);
    if(pthread_mutex_lock(thread_func_args->mutex) != 0){
        ERROR_LOG("thread: %lu  wait_to_obtain: %d   wait_to_release: %d",pthread_self(),
         thread_func_args->wait_to_obtain_us, thread_func_args->wait_to_release_us);
        return thread_param;
    }
    DEBUG_LOG(" Locked mutex  thread: %lu wait_to_obtain: %d   wait_to_release: %d",pthread_self(), 
    thread_func_args->wait_to_obtain_us, thread_func_args->wait_to_release_us);
    //wait, 
    usleep(thread_func_args->wait_to_release_us);
    //release mutex as described by thread_data structure
    pthread_mutex_unlock(thread_func_args->mutex);
    DEBUG_LOG(" Unlocked mutex thread: %lu wait_to_obtain: %d   wait_to_release: %d",pthread_self(), 
    thread_func_args->wait_to_obtain_us, thread_func_args->wait_to_release_us);
    thread_func_args->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data *data;
    
     // allocate memory for thread_data,
      data = (struct thread_data*)malloc(sizeof(struct thread_data));
     // setup mutex and wait arguments, 
     data->mutex = mutex;
     data->wait_to_obtain_us = wait_to_obtain_ms;
     data->wait_to_release_us = wait_to_release_ms;
     data->thread_complete_success = false;

     DEBUG_LOG(" Creating thread  wait_to_obtain: %d   wait_to_release: %d", 
    wait_to_obtain_ms, wait_to_release_ms);
     // pass thread_data to created thread using threadfunc() as entry point.
    int rc = pthread_create(thread, NULL, threadfunc, data);
    if( rc != 0){
        free(data);
        return false;
    }
    DEBUG_LOG(" Started thread: %lu wait_to_obtain: %d   wait_to_release: %d",*thread, 
    data->wait_to_obtain_us, data->wait_to_release_us);
    // return true if successful. 
    return true;
}

