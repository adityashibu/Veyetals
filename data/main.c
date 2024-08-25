#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif

#include <stdio.h>

void get_cpu_memory_usage()
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

int main()
{
    get_cpu_memory_usage();
    return 0;
}