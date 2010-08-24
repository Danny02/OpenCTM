/* Threads.c -- multithreading library
2008-08-05
Igor Pavlov
Public domain */

#include "Threads.h"

#ifdef _USE_WIN32_THREADS
  #include <process.h>
#else
  #include <unistd.h>
#endif

//============================================================================
// Private helper functions
//============================================================================

#ifdef _USE_WIN32_THREADS
static WRes GetError()
{
  DWORD res = GetLastError();
  return (res) ? (WRes)(res) : 1;
}

static WRes HandleToWRes(HANDLE h) { return (h != 0) ? 0 : GetError(); }
static WRes BOOLToWRes(BOOL v) { return v ? 0 : GetError(); }

static WRes MyCloseHandle(HANDLE *h)
{
  if (*h != NULL)
    if (!CloseHandle(*h))
      return GetError();
  *h = NULL;
  return 0;
}

static WRes WaitObject(HANDLE h)
{
  return (WRes)WaitForSingleObject(h, INFINITE);
}

static WRes Event_Create(CEvent *p, BOOL manualReset, int initialSignaled)
{
  p->handle = CreateEvent(NULL, manualReset, (initialSignaled ? TRUE : FALSE), NULL);
  return HandleToWRes(p->handle);
}

WRes Semaphore_Release(CSemaphore *p, LONG releaseCount, LONG *previousCount)
{
  return BOOLToWRes(ReleaseSemaphore(p->handle, releaseCount, previousCount));
}
#endif // _USE_WIN32_THREADS


//============================================================================
// Public functions
//============================================================================

//----------------------------------------------------------------------------
// CThread - thread class
//----------------------------------------------------------------------------

#ifdef _USE_POSIX_THREADS
typedef void *(*_thread_fun_ptr)(void*);
#endif

WRes Thread_Create(CThread *thread, THREAD_FUNC_RET_TYPE (THREAD_FUNC_CALL_TYPE *startAddress)(void *), void *parameter)
{
#ifdef _USE_WIN32_THREADS
  unsigned threadId; /* Windows Me/98/95: threadId parameter may not be NULL in _beginthreadex/CreateThread functions */
  thread->handle =
    /* CreateThread(0, 0, startAddress, parameter, 0, &threadId); */
    (HANDLE)_beginthreadex(NULL, 0, startAddress, parameter, 0, &threadId);
    /* maybe we must use errno here, but probably GetLastError() is also OK. */
  return HandleToWRes(thread->handle);
#else
  if (pthread_create(&thread->handle, NULL, (_thread_fun_ptr)startAddress, parameter) != 0)
    thread->handle = 0;
  return (thread->handle != 0 ? 0 : 1);
#endif
}

WRes Thread_Wait(CThread *thread)
{
#ifdef _USE_WIN32_THREADS
  if (thread->handle == NULL)
    return 1;
  return WaitObject(thread->handle);
#else
  if (thread->handle == 0)
    return 1;
  pthread_join(thread->handle, NULL);
  return 0;
#endif
}

WRes Thread_Close(CThread *thread)
{
#ifdef _USE_WIN32_THREADS
  return MyCloseHandle(&thread->handle);
#else
  int err = pthread_cancel(thread->handle);
  thread->handle = 0;
  return err;
#endif
}

unsigned int Thread_HardwareConcurrency(void)
{
  long nproc;
#if defined(_USE_WIN32_THREADS)
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  nproc = (long) si.dwNumberOfProcessors;
#elif defined(_SC_NPROCESSORS_ONLN)
  nproc = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_SC_NPROC_ONLN)
  nproc = sysconf(_SC_NPROC_ONLN);
#else
  nproc = 1;
#endif
  return nproc > 0 ? (unsigned int) nproc : 1;
}

//----------------------------------------------------------------------------
// CEvent - event classes
//----------------------------------------------------------------------------

WRes ManualResetEvent_Create(CManualResetEvent *p, int initialSignaled)
{
#ifdef _USE_WIN32_THREADS
  return Event_Create(p, TRUE, initialSignaled);
#else
  // FIXME!
  (void) p;
  (void) initialSignaled;
  return 1;
#endif
}

WRes ManualResetEvent_CreateNotSignaled(CManualResetEvent *p)
{
#ifdef _USE_WIN32_THREADS
  return ManualResetEvent_Create(p, 0);
#else
  // FIXME!
  (void) p;
  return 1;
#endif
}

WRes AutoResetEvent_Create(CAutoResetEvent *p, int initialSignaled)
{
#ifdef _USE_WIN32_THREADS
  return Event_Create(p, FALSE, initialSignaled);
#else
  // FIXME!
  (void) p;
  (void) initialSignaled;
  return 1;
#endif
}

WRes AutoResetEvent_CreateNotSignaled(CAutoResetEvent *p)
{
#ifdef _USE_WIN32_THREADS
  return AutoResetEvent_Create(p, 0);
#else
  // FIXME!
  (void) p;
  return 1;
#endif
}

WRes Event_Set(CEvent *p)
{
#ifdef _USE_WIN32_THREADS
  return BOOLToWRes(SetEvent(p->handle));
#else
  // FIXME!
  (void) p;
  return 1;
#endif
}

WRes Event_Reset(CEvent *p)
{
#ifdef _USE_WIN32_THREADS
  return BOOLToWRes(ResetEvent(p->handle));
#else
  // FIXME!
  (void) p;
  return 1;
#endif
}

WRes Event_Wait(CEvent *p)
{
#ifdef _USE_WIN32_THREADS
  return WaitObject(p->handle);
#else
  // FIXME!
  (void) p;
  return 1;
#endif
}

WRes Event_Close(CEvent *p)
{
#ifdef _USE_WIN32_THREADS
  return MyCloseHandle(&p->handle);
#else
  // FIXME!
  (void) p;
  return 1;
#endif
}

//----------------------------------------------------------------------------
// CSemaphore - semaphore class
//----------------------------------------------------------------------------

WRes Semaphore_Create(CSemaphore *p, UInt32 initiallyCount, UInt32 maxCount)
{
#ifdef _USE_WIN32_THREADS
  p->handle = CreateSemaphore(NULL, (LONG)initiallyCount, (LONG)maxCount, NULL);
  return HandleToWRes(p->handle);
#else
  if (pthread_mutex_init(&p->lock, NULL) == 0)
  {
    if (pthread_cond_init(&p->nonZero, NULL) == 0)
    {
      p->count = initiallyCount;
      p->maxCount = maxCount;
      p->init = 1;
      return 0;
    }
    else
      pthread_mutex_destroy(&p->lock);
  }
  return 1;
#endif
}

WRes Semaphore_ReleaseN(CSemaphore *p, UInt32 releaseCount)
{
#ifdef _USE_WIN32_THREADS
  return Semaphore_Release(p, (LONG)releaseCount, NULL);
#else
  pthread_mutex_lock(&p->lock);
  if ((p->count + releaseCount) > p->maxCount)
  {
    pthread_mutex_unlock(&p->lock);
    return 1;
  }
  p->count += releaseCount;
  pthread_mutex_unlock(&p->lock);
  pthread_cond_broadcast(&p->nonZero);
  return 0;
#endif
}

WRes Semaphore_Release1(CSemaphore *p)
{
  return Semaphore_ReleaseN(p, 1);
}

WRes Semaphore_Wait(CSemaphore *p)
{
#ifdef _USE_WIN32_THREADS
  return WaitObject(p->handle);
#else
  pthread_mutex_lock(&p->lock);
  while (p->count == 0)
    pthread_cond_wait(&p->nonZero, &p->lock);
  -- p->count;
  pthread_mutex_unlock(&p->lock);
  return 0;
#endif
}

WRes Semaphore_Close(CSemaphore *p)
{
#ifdef _USE_WIN32_THREADS
  return MyCloseHandle(&p->handle);
#else
  if (p->init)
  {
    pthread_cond_destroy(&p->nonZero);
    pthread_mutex_destroy(&p->lock);
    p->init = 0;
  }
  return 0;
#endif
}


//----------------------------------------------------------------------------
// CCriticalSection - mutex class
//----------------------------------------------------------------------------

WRes CriticalSection_Init(CCriticalSection *p)
{
#ifdef _USE_WIN32_THREADS
  /* InitializeCriticalSection can raise only STATUS_NO_MEMORY exception */
  __try
  {
    InitializeCriticalSection(p);
    /* InitializeCriticalSectionAndSpinCount(p, 0); */
  }
  __except (EXCEPTION_EXECUTE_HANDLER) { return 1; }
  return 0;
#else
  return pthread_mutex_init(p, NULL);
#endif
}
