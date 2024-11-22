#ifndef THREAD_H
#define THREAD_H

#define THREAD_COUNT 4
#define WORK_BUFFER_SIZE 8

typedef void WorkItemCallback_Fn(void*);
typedef struct {
  WorkItemCallback_Fn* callback;
  void* data;
} WorkItem;

typedef struct {
  HANDLE threads[THREAD_COUNT];
  HANDLE semaphore;
  WorkItem work_buffer[WORK_BUFFER_SIZE];
  volatile LONG work_buffer_count;
  volatile LONG in_flight_requests;
} ThreadPool;

Status ThreadPoolInit(ThreadPool* thread_pool);
Status ThreadPoolClose(ThreadPool* thread_pool);
bool ThreadPoolAddWorkItem(ThreadPool* thread_pool, WorkItem item);

#endif
