#ifdef _WIN32
#include <windows.h>
#include <Psapi.h>
#elif __linux__
#include <unistd.h>
#endif

#include <stdio.h>
#include <nvml.h>
#include <time.h>

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

void get_cpu_temperature()
{
    HRESULT hres;
    IWbemLocator *pLoc = NULL;
    IWbemServices *pSvc = NULL;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        printf("Failed to initialize COM library. Error code = 0x%X\n", hres);
        return;
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
                                RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL, EOAC_NONE, NULL);
    if (FAILED(hres))
    {
        printf("Failed to initialize security. Error code = 0x%X\n", hres);
        CoUninitialize();
        return;
    }

    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
    if (FAILED(hres))
    {
        printf("Failed to create IWbemLocator object. Error code = 0x%X\n", hres);
        CoUninitialize();
        return;
    }

    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),
                               NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hres))
    {
        printf("Could not connect. Error code = 0x%X\n", hres);
        pLoc->Release();
        CoUninitialize();
        return;
    }

    hres = CoSetProxyBlanket(pSvc,
                             RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                             RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
                             NULL, EOAC_NONE);

    if (FAILED(hres))
    {
        printf("Could not set proxy blanket. Error code = 0x%X\n", hres);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IEnumWbemClassObject *pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_PerfFormattedData_Counters_ThermalZoneInformation"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        printf("Query for CPU temperature failed. Error code = 0x%X\n", hres);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        hr = pclsObj->Get(L"Temperature", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr))
        {
            printf("CPU Temperature: %.1f °C\n", (vtProp.intVal - 273.15));
        }
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
}


void get_gpu_usage()
{
    nvmlReturn_t result;
    nvmlDevice_t device;
    unsigned int temp;
    char name[NVML_DEVICE_NAME_BUFFER_SIZE];

    result = nvmlInit();
    if (result != NVML_SUCCESS)
    {
        printf("Failed to initialize NVML: %s\n", nvmlErrorString(result));
        return;
    }

    result = nvmlDeviceGetHandleByIndex_v2(0, &device);
    if (result != NVML_SUCCESS)
    {
        printf("Failed to get handle for device: %s\n", nvmlErrorString(result));
        nvmlShutdown();
        return;
    }

    result = nvmlDeviceGetName(device, name, sizeof(name));
    if (result != NVML_SUCCESS)
    {
        printf("Failed to get GPU Name: %s\n", nvmlErrorString(result));
        nvmlShutdown();
        return;
    }

    printf("GPU Name: %s\n", name);

    nvmlUtilization_t utilization;
    result = nvmlDeviceGetUtilizationRates(device, &utilization);
    if (result != NVML_SUCCESS)
    {
        printf("Failed to get GPU utilization rates: %s\n", nvmlErrorString(result));
    }
    else
    {
        printf("GPU Usage: %u%%\n", utilization.gpu);
    }

    nvmlMemory_t memory;
    result = nvmlDeviceGetMemoryInfo(device, &memory);
    if (result != NVML_SUCCESS)
    {
        printf("Failed to get GPU memory info: %s\n", nvmlErrorString(result));
    }
    else
    {
        printf("GPU Memory Usage: %llu MB / %llu MB\n", memory.used / (1024 * 1024), memory.total / (1024 * 1024));
    }

    result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp);
    if (result != NVML_SUCCESS)
    {
        printf("Failed to get GPU Temperatures: %s\n", nvmlErrorString(result));
    }
    else
    {
        printf("GPU Temperature: %u C\n", temp);
    }

    nvmlShutdown();
}

void get_current_time()
{
    time_t t;
    struct tm *tm_info;

    time(&t);
    tm_info = localtime(&t);

    char day_buffer[10];
    char date_buffer[26];

    strftime(day_buffer, 10, "%A", tm_info);
    strftime(date_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    printf("Day: %s\n", day_buffer);
    printf("Date and Time: %s\n", date_buffer);
}

int main()
{
    setbuf(stdout, NULL);
    while (1)
    {
        get_current_time();
        get_memory_usage();
        get_cpu_usage();
        get_cpu_temperature();
        get_gpu_usage();
        Sleep(1000);
        printf("--------------------------------------------\n");
    }
    return 0;
}

// cl /Fe:main.exe main.c /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6\include" /link /MACHINE:x64 /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.6\lib\x64" nvml.lib
// main.exe