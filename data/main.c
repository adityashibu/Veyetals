#ifdef _WIN32
#include <windows.h>
#include <Psapi.h>
#elif __linux__
#include <unistd.h>
#endif

#include <stdio.h>

void get_memory_usage()
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    printf("Memory Usage: %ld%%\n", memInfo.dwMemoryLoad);
    printf("Total Physical Memory: %lld MB\n", memInfo.ullTotalPhys / (1024 * 1024));
    printf("Available Physical Memory: %lld MB\n", memInfo.ullAvailPhys / (1024 * 1024));
#elif __linux__
    // Add linux code here
#endif
}

void get_cpu_usage()
{
    FILETIME idleTime, kernelTime, userTime;
    static FILETIME prev_idleTime = {0}, prev_kernelTime = {0}, prev_userTime = {0};

    if (GetSystemTimes(&idleTime, &kernelTime, &userTime))
    {
        ULARGE_INTEGER idle, kernel, user;
        ULARGE_INTEGER prev_idle, prev_kernel, prev_user;
        ULARGE_INTEGER total_time, prev_total_time;
        double cpu_usage;

        idle.QuadPart = ((ULARGE_INTEGER *)&idleTime)->QuadPart;
        kernel.QuadPart = ((ULARGE_INTEGER *)&kernelTime)->QuadPart;
        user.QuadPart = ((ULARGE_INTEGER *)&userTime)->QuadPart;

        prev_idle.QuadPart = ((ULARGE_INTEGER *)&prev_idleTime)->QuadPart;
        prev_kernel.QuadPart = ((ULARGE_INTEGER *)&prev_kernelTime)->QuadPart;
        prev_user.QuadPart = ((ULARGE_INTEGER *)&prev_userTime)->QuadPart;

        ULONGLONG idle_diff = idle.QuadPart - prev_idle.QuadPart;
        ULONGLONG kernel_diff = kernel.QuadPart - prev_kernel.QuadPart;
        ULONGLONG user_diff = user.QuadPart - prev_user.QuadPart;

        ULONGLONG total_diff = kernel_diff + user_diff;
        ULONGLONG total_idle = idle_diff;

        if (total_diff == 0)
        {
            cpu_usage = 0;
        }
        else
        {
            cpu_usage = (double)(total_diff - total_idle) / total_diff * 1000.0;
        }

        printf("CPU Usage: %.1f%%\n", cpu_usage);

        prev_idleTime = idleTime;
        prev_kernelTime = kernelTime;
        prev_userTime = userTime;
    }
}

int main()
{
    while (1)
    {
        get_memory_usage();
        get_cpu_usage();
        Sleep(1000);
    }
    return 0;
}