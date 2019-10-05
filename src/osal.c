#include <stdio.h>
#include <windows.h>
#include "osal.h"

skhl_handle event_init(event_attr *desc)
{
    HANDLE event;

    event = CreateEvent(
            NULL,               // default security attributes
            FALSE,              // manual-reset event
            FALSE,              // initial state is nonsignaled
            TEXT(desc->name)    // object name
            );

    return (skhl_handle )event;
}

skhl_result event_post(skhl_handle event_handle)
{
    SetEvent((HANDLE)event_handle);
    return 0;
}

skhl_result event_wait(skhl_handle event_handle)
{
    DWORD dwWaitResult;
    MSG msgCurrent;
    HANDLE event_local = (HANDLE)event_handle;

    while (PeekMessage (&msgCurrent, NULL, 0, 0, PM_REMOVE))
    {
        if (WM_QUIT == msgCurrent.message)
        {
            return 1;
        }
       DispatchMessage (&msgCurrent);
    }

    dwWaitResult = MsgWaitForMultipleObjects(
                        1,
                        &event_local,           // event handle list
                        FALSE,
                        INFINITE,               // indefinite wait
                        QS_ALLINPUT);

    if (dwWaitResult != WAIT_OBJECT_0)
    {
        log_err("Wait error (%ld)\n", GetLastError());
        return -1;
    }

    return 0;
}

skhl_result event_destory(skhl_handle event_handle)
{
    CloseHandle((HANDLE)event_handle);
    return 0;
}

skhl_handle mutex_init(mutex_attr *desc)
{
    HANDLE mutex;

    if (NULL == desc)
    {
        return NULL;
    }

    mutex = CreateMutex(
            NULL,                       // default security attributes
            FALSE,                      // initially not owned
            TEXT(desc->name)            // unnamed mutex
            );

    return (skhl_handle )mutex;
}

skhl_result mutex_lock(skhl_handle mutex_handle)
{
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
            (HANDLE)mutex_handle,       // handle to mutex
            INFINITE);                  // no time-out interval

    if (dwWaitResult != WAIT_OBJECT_0)
    {
        log_err("Wait error (%ld)\n", GetLastError());
        return -1;
    }

    return 0;
}

skhl_result mutex_unlock(skhl_handle mutex_handle)
{
    // Release ownership of the mutex object
    if (!ReleaseMutex((HANDLE)mutex_handle))
    {
        log_err("Thread %ld release mutex error!\n",
                        GetCurrentThreadId());
    }

    return 0;
}

skhl_result mutex_destory(skhl_handle mutex_handle)
{
    CloseHandle((HANDLE)mutex_handle);
    return 0;
}

skhl_handle task_init(task_attr *desc)
{
    HANDLE task;

    if (desc == NULL)
    {
        return NULL;
    }

    task = CreateThread(
            NULL,                   // default security
            desc->stack_size,       // default stack size
            desc->fn,               // name of the thread function
            desc->arg,              // no thread parameters
            0,                      // default startup flags
            &desc->task_id          // thread id
            );
    SetThreadPriority(task, desc->prio);

    return (skhl_handle)task;
}

skhl_result task_destory(skhl_handle task_handle)
{
    CloseHandle((HANDLE)task_handle);
    task_handle = NULL;
    return 0;
}

skhl_handle file_init(file_attr *desc)
{
    HANDLE file;

    if (NULL == desc)
    {
        return NULL;
    }

    file = CreateFile(TEXT(desc->name),             // serial com port.
            desc->access,                           // GENERIC_READ | GENERIC_WRITE
            desc->share_mode,                       // share mode, no share, should be zero.
            NULL,                                   // SecurityAttributes
            desc->creation,                         // OPEN_EXISTING, open but not create.
            desc->flag,                             // FILE_ATTRIBUTE_NORMAL
            NULL);

    return (skhl_handle)file;
}

skhl_result file_read(skhl_handle file, uint8_t *buff, uint32_t size, int32_t *real_size)
{
    ReadFile((HANDLE)file,
        buff,
        (DWORD)size,
        (LPDWORD)real_size,
        NULL);

    if (*real_size == 0)
    {
        return -1;
    }

    return 0;
}

skhl_result file_write(skhl_handle file, uint8_t *buff, uint32_t size, int32_t *real_size)
{
    WriteFile((HANDLE)file,
        buff,
        (DWORD)size,
        (LPDWORD)real_size,//用于保存实际写入字节数的存储区域的指针
        NULL);

    if (*real_size != size)
    {
        return -1;
    }

    return 0;
}

skhl_result get_file_size(skhl_handle file, uint32_t *file_size)
{
    GetFileSize((HANDLE)file,
                (LPDWORD)file_size
                );

    return 0;
}

skhl_result file_close(skhl_handle file)
{
    CloseHandle((HANDLE)file);
    return 0;
}

void skhl_sleep(uint32_t ms)
{
    Sleep(ms);
}

