#ifndef _SIMU_UTILITY_THREAD_H_
#define _SIMU_UTILITY_THREAD_H_
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
    #include <winsock2.h>
    #include <process.h>
    #include <sys/locking.h>
    #include <Windows.h>
    #define THREAD_SPIN_COUNT 2000

    typedef HANDLE thread_t;
    #define THREAD_FUNC unsigned __stdcall
    #define THREAD_CREATE(threadvar, fn, arg) do { \
        uintptr_t threadhandle = _beginthreadex(NULL,0,fn,(arg),0,NULL); \
        (threadvar) = (thread_t) threadhandle; \
    } while (0)
    #define THREAD_JOIN(th) WaitForSingleObject(th, INFINITE)
    #define THREAD_RETURN return (0)

    #define THREAD_SLEEP_MS(TM) Sleep(TM)
#elif defined(__GNUC__) || defined(__clang__)
    #include <pthread.h>
    #include <errno.h>
    #include <unistd.h>

    typedef pthread_t thread_t;
    #define THREAD_FUNC void*
    #define THREAD_CREATE(threadvar, fn, arg) \
        pthread_create(&(threadvar), NULL, fn, arg)
    #define THREAD_JOIN(th) pthread_join(th, NULL)
    #define THREAD_RETURN pthread_exit (NULL); return NULL

    #define THREAD_SLEEP_MS(TM) ((TM > 1000)? sleep(TM / 1000): usleep(0)); usleep((TM % 1000) * 1000)
#endif

#ifdef __cplusplus
}
#endif

#endif // THREAD_H_


