#include "thread.h"

static WorkItem* GetNextWorkItem(ThreadPool* thread_pool) {
  int32_t old_count;
  int32_t new_count;
  int32_t value;
  do {
    if (thread_pool->work_buffer_count <= 0) { return NULL; }
    old_count = thread_pool->work_buffer_count;
    new_count = old_count - 1;
    value = InterlockedCompareExchange(
        &thread_pool->work_buffer_count, new_count, old_count);
  } while (value != old_count);
  return &thread_pool->work_buffer[new_count];
}

static bool MaybeDoWork(ThreadPool* thread_pool) {
  WorkItem* work_item = GetNextWorkItem(thread_pool);
  if (work_item == NULL) { return false; }

  InterlockedIncrement((volatile LONG*) &thread_pool->in_flight_requests);
  (work_item->callback)(work_item->data);
  InterlockedDecrement((volatile LONG*) &thread_pool->in_flight_requests);

  return true;
}

static DWORD WINAPI ThreadProc(LPVOID data) {
  ThreadPool* thread_pool = (ThreadPool*) data;
  DWORD thread_id = GetCurrentThreadId();
  LOG_INFO("Thread initialized with id: %d", thread_id);
  for (;;) {
    if (!MaybeDoWork(thread_pool)) {
      WaitForSingleObjectEx(thread_pool->semaphore, INFINITE, FALSE);
    }
  }
}

Status ThreadPoolInit(ThreadPool* thread_pool) {
  *thread_pool = (ThreadPool) {};

  HANDLE semaphore = CreateSemaphoreEx(NULL, 0, THREAD_COUNT, 0, 0, SEMAPHORE_ALL_ACCESS);
  if (semaphore == NULL) { return INTERNAL("Unable to create semaphore! %d", GetLastError()); }
  thread_pool->semaphore = semaphore;

  for (int32_t i = 0; i < THREAD_COUNT; i++) {
    HANDLE thread = CreateThread(NULL, 0, ThreadProc, thread_pool, 0, NULL);
    if (thread == NULL) { return INTERNAL("Unable to create thread! %d", GetLastError()); }
    thread_pool->threads[i] = thread;
  }

  LOG_INFO("Successfully initialized thread pool.");
  return OK();
}

Status ThreadPoolClose(ThreadPool* thread_pool) {
  for (int32_t i = 0; i < THREAD_COUNT; i++) {
    HANDLE thread = thread_pool->threads[i];
    BOOL success = CloseHandle(thread);
    if (!success) {
      return INTERNAL("Unable to close thread: %d with error code: %d", thread, GetLastError());
    }
  }

  LOG_INFO("Successfully closed thread pool.");
  return OK();
}

bool ThreadPoolAddWorkItem(ThreadPool* thread_pool, WorkItem item) {
  int32_t old_count;
  int32_t new_count;
  int32_t value;
  do {
    if (thread_pool->work_buffer_count > WORK_BUFFER_SIZE) { return false; }
    old_count = thread_pool->work_buffer_count;
    new_count = old_count + 1;
    value = InterlockedCompareExchange(
        &thread_pool->work_buffer_count, new_count, old_count);
  } while (value != old_count);

  thread_pool->work_buffer[old_count] = item;
  ReleaseSemaphore(thread_pool->semaphore, 1, NULL);
  return true;
}
