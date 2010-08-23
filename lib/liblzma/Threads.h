/* Threads.h -- multithreading library
2008-11-22 : Igor Pavlov : Public domain */

#ifndef __7Z_THRESDS_H
#define __7Z_THRESDS_H

#include "Types.h"

#ifdef _WIN32
  #define _USE_WIN32_THREADS
#else
  #define _USE_POSIX_THREADS
#endif

#ifdef _USE_WIN32_THREADS
  #include <windows.h>
#else
  #include <pthread.h>
#endif


typedef struct _CThread
{
#ifdef _USE_WIN32_THREADS
  HANDLE handle;
#else
  pthread_t handle;
#endif
} CThread;

#define Thread_Construct(thread) (thread)->handle = NULL
#define Thread_WasCreated(thread) ((thread)->handle != NULL)
 
typedef unsigned THREAD_FUNC_RET_TYPE;
#define THREAD_FUNC_CALL_TYPE MY_STD_CALL
#define THREAD_FUNC_DECL THREAD_FUNC_RET_TYPE THREAD_FUNC_CALL_TYPE

WRes Thread_Create(CThread *thread, THREAD_FUNC_RET_TYPE (THREAD_FUNC_CALL_TYPE *startAddress)(void *), LPVOID parameter);
WRes Thread_Wait(CThread *thread);
WRes Thread_Close(CThread *thread);

typedef struct _CEvent
{
  HANDLE handle;
} CEvent;

typedef CEvent CAutoResetEvent;
typedef CEvent CManualResetEvent;

#define Event_Construct(event) (event)->handle = NULL
#define Event_IsCreated(event) ((event)->handle != NULL)

WRes ManualResetEvent_Create(CManualResetEvent *event, int initialSignaled);
WRes ManualResetEvent_CreateNotSignaled(CManualResetEvent *event);
WRes AutoResetEvent_Create(CAutoResetEvent *event, int initialSignaled);
WRes AutoResetEvent_CreateNotSignaled(CAutoResetEvent *event);
WRes Event_Set(CEvent *event);
WRes Event_Reset(CEvent *event);
WRes Event_Wait(CEvent *event);
WRes Event_Close(CEvent *event);


typedef struct _CSemaphore
{
  HANDLE handle;
} CSemaphore;

#define Semaphore_Construct(p) (p)->handle = NULL

WRes Semaphore_Create(CSemaphore *p, UInt32 initiallyCount, UInt32 maxCount);
WRes Semaphore_ReleaseN(CSemaphore *p, UInt32 num);
WRes Semaphore_Release1(CSemaphore *p);
WRes Semaphore_Wait(CSemaphore *p);
WRes Semaphore_Close(CSemaphore *p);

#ifdef _USE_WIN32_THREADS
typedef CRITICAL_SECTION CCriticalSection;
#else
typedef pthread_mutex_t CCriticalSection;
#endif

#ifdef _USE_WIN32_THREADS
WRes CriticalSection_Init(CCriticalSection *p);
#define CriticalSection_Delete(p) DeleteCriticalSection(p)
#define CriticalSection_Enter(p) EnterCriticalSection(p)
#define CriticalSection_Leave(p) LeaveCriticalSection(p)
#else
WRes CriticalSection_Init(CCriticalSection *p);
#define CriticalSection_Delete(p) pthread_mutex_destroy(p)
#define CriticalSection_Enter(p) pthread_mutex_lock(p)
#define CriticalSection_Leave(p) pthread_mutex_unlock(p)
#endif

#endif

