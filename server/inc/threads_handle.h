#ifndef __THREADS_HANDLE_H__
#define __THREADS_HANDLE_H__

#include "common.h"

pthread_t* handler_create(void);

void handler_destroy(pthread_t tid);

void handler_destroy_all(void);

void handler_wait_all(void);

#endif // __THREADS_HANDLE_H__