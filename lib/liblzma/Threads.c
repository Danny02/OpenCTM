/* Threads.c -- multithreading library
2008-08-05
Igor Pavlov
Public domain */

#include "Threads.h"

#ifdef _USE_WIN32_THREADS
  #include <process.h>
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
  // FIXME!
  (void) p;
  (void) initiallyCount;
  (void) maxCount;
  return 1;
#endif
}

WRes Semaphore_ReleaseN(CSemaphore *p, UInt32 releaseCount)
{
#ifdef _USE_WIN32_THREADS
  return Semaphore_Release(p, (LONG)releaseCount, NULL);
#else
  // FIXME!
  (void) p;
  (void) releaseCount;
  return 1;
#endif
}

WRes Semaphore_Release1(CSemaphore *p)
{
#ifdef _USE_WIN32_THREADS
  return Semaphore_ReleaseN(p, 1);
#else
  // FIXME!
  (void) p;
  return 1;
#endif
}

WRes Semaphore_Wait(CSemaphore *p)
{
#ifdef _USE_WIN32_THREADS
  return WaitObject(p->handle);
#else
  // FIXME!
  (void) p;
  return 1;
#endif
}

WRes Semaphore_Close(CSemaphore *p)
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
