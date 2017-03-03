/*
 Copyright (C) 2015-2017 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#ifndef MyCORE_THREAD_H
#define MyCORE_THREAD_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "mycore/myosi.h"

#ifndef MyCORE_BUILD_WITHOUT_THREADS

#if !defined(IS_OS_WINDOWS)
#   include <pthread.h>
#   include <semaphore.h>
#endif

#include <time.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <errno.h>

#include "mycore/mystring.h"

#endif /* MyCORE_BUILD_WITHOUT_THREADS */

#define MyTHREAD_SEM_NAME "mycore"

/* functions */
typedef void (*mythread_callback_before_join_f)(mythread_t* mythread);
typedef void (*mythread_process_f)(void* arg);
typedef void (*mythread_work_f)(mythread_id_t thread_id, void* arg);

#ifdef MyCORE_BUILD_WITHOUT_THREADS

struct mythread {
    int sys_last_error;
};

#else /* MyCORE_BUILD_WITHOUT_THREADS */

void mythread_function_queue_stream(void *arg);
void mythread_function_queue_batch(void *arg);
void mythread_function(void *arg);

// thread
struct mythread_context {
    mythread_id_t id;
    
#if defined(IS_OS_WINDOWS)
    HANDLE mutex;
#else
    pthread_mutex_t *mutex;
#endif
    
    size_t sem_name_size;
    
    mythread_work_f func;
    
    volatile size_t t_count;
    volatile mythread_thread_opt_t opt;
    
    mythread_t *mythread;
    unsigned int status;
};

struct mythread_list {
#if defined(IS_OS_WINDOWS)
    HANDLE pth;
#else
    pthread_t pth;
#endif
    mythread_context_t data;
    mythread_process_f process_func;
};

struct mythread_workers_list {
    mythread_list_t *list;
    size_t count;
};

struct mythread {
    mythread_list_t *pth_list;
    size_t pth_list_length;
    size_t pth_list_size;
    size_t pth_list_root;
    
    void *context;
    
    char  *sem_prefix;
    size_t sem_prefix_length;
    
#if !defined(IS_OS_WINDOWS)
    pthread_attr_t *attr;
#endif
    
    int sys_last_error;
    
    mythread_id_t batch_first_id;
    mythread_id_t batch_count;
    
    volatile mythread_thread_opt_t stream_opt;
    volatile mythread_thread_opt_t batch_opt;
};

mythread_id_t myhread_create_stream(mythread_t *mythread, mythread_process_f process_func, mythread_work_f func, mythread_thread_opt_t opt, mystatus_t *status);
mythread_id_t myhread_create_batch(mythread_t *mythread, mythread_process_f process_func, mythread_work_f func, mythread_thread_opt_t opt, mystatus_t *status, size_t count);

void mycore_thread_nanosleep(const struct timespec *tomeout);

#endif /* MyCORE_BUILD_WITHOUT_THREADS */

mythread_t * mythread_create(void);
mystatus_t mythread_init(mythread_t *mythread, const char *sem_prefix, size_t thread_count);
void mythread_clean(mythread_t *mythread);
mythread_t * mythread_destroy(mythread_t *mythread, mythread_callback_before_join_f before_join, bool self_destroy);

void mythread_stream_quit_all(mythread_t *mythread);
void mythread_batch_quit_all(mythread_t *mythread);

void mythread_stream_stop_all(mythread_t *mythread);
void mythread_batch_stop_all(mythread_t *mythread);

void mythread_stop_all(mythread_t *mythread);
void mythread_queue_wait_all_for_done(mythread_t *mythread);
void mythread_resume_all(mythread_t *mythread);
void mythread_suspend_all(mythread_t *mythread);
unsigned int mythread_check_status(mythread_t *mythread);

// queue
struct mythread_queue_node {
    void* context;
    void* args;
    
    mythread_queue_node_t* prev;
};

struct mythread_queue_thread_param {
    volatile size_t use;
};

struct mythread_queue_list_entry {
    mythread_queue_list_entry_t *next;
    mythread_queue_list_entry_t *prev;
    mythread_queue_t *queue;
    mythread_queue_thread_param_t *thread_param;
};

struct mythread_queue_list {
    mythread_queue_list_entry_t *first;
    mythread_queue_list_entry_t *last;
    
    volatile size_t count;
};

struct mythread_queue {
    mythread_queue_node_t **nodes;
    
    size_t nodes_pos;
    size_t nodes_pos_size;
    size_t nodes_length;
    
    volatile size_t nodes_uses;
    volatile size_t nodes_size;
    volatile size_t nodes_root;
};

mythread_queue_t * mythread_queue_create(size_t size, mystatus_t *status);
void mythread_queue_clean(mythread_queue_t* queue);
mythread_queue_t * mythread_queue_destroy(mythread_queue_t* token);

void mythread_queue_node_clean(mythread_queue_node_t* qnode);

size_t mythread_queue_count_used_node(mythread_queue_t* queue);
mythread_queue_node_t * mythread_queue_get_first_node(mythread_queue_t* queue);
mythread_queue_node_t * mythread_queue_get_prev_node(mythread_queue_node_t* qnode);
mythread_queue_node_t * mythread_queue_get_current_node(mythread_queue_t* queue);
mythread_queue_node_t * mythread_queue_node_malloc(mythread_t *mythread, mythread_queue_t* queue, mystatus_t *status);
mythread_queue_node_t * mythread_queue_node_malloc_limit(mythread_t *mythread, mythread_queue_t* queue, size_t limit, mystatus_t *status);

#ifndef MyCORE_BUILD_WITHOUT_THREADS

mythread_queue_list_t * mythread_queue_list_create(mystatus_t *status);
void mythread_queue_list_destroy(mythread_queue_list_t* queue_list);

size_t mythread_queue_list_get_count(mythread_queue_list_t* queue_list);

mythread_queue_list_entry_t * mythread_queue_list_entry_push(mythread_t *mythread, mythread_queue_t *queue, mystatus_t *status);
mythread_queue_list_entry_t * mythread_queue_list_entry_delete(mythread_t *mythread, mythread_queue_list_entry_t *entry, bool destroy_queue);
void mythread_queue_list_entry_clean(mythread_t *mythread, mythread_queue_list_entry_t *entry);
void mythread_queue_list_entry_wait_for_done(mythread_t *mythread, mythread_queue_list_entry_t *entry);
    
mythread_queue_node_t * mythread_queue_node_malloc_round(mythread_t *mythread, mythread_queue_list_entry_t *entry, mystatus_t *status);

#endif /* MyCORE_BUILD_WITHOUT_THREADS */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined(__mycore__mycore_thread__) */
