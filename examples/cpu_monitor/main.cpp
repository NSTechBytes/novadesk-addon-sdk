#include <NovadeskAPI/novadesk_addon.h>
#include <thread>
#include <atomic>

// --- CPU Usage Helper ---
static FILETIME g_prevIdleTime;
static FILETIME g_prevKernelTime;
static FILETIME g_prevUserTime;

ULONGLONG SubtractTimes(const FILETIME& a, const FILETIME& b) {
    ULARGE_INTEGER la, lb;
    la.LowPart = a.dwLowDateTime; la.HighPart = a.dwHighDateTime;
    lb.LowPart = b.dwLowDateTime; lb.HighPart = b.dwHighDateTime;
    return la.QuadPart - lb.QuadPart;
}

double GetCPUUsage() {
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) return 0.0;

    ULONGLONG idleDiff = SubtractTimes(idleTime, g_prevIdleTime);
    ULONGLONG kernelDiff = SubtractTimes(kernelTime, g_prevKernelTime);
    ULONGLONG userDiff = SubtractTimes(userTime, g_prevUserTime);

    g_prevIdleTime = idleTime;
    g_prevKernelTime = kernelTime;
    g_prevUserTime = userTime;

    ULONGLONG totalDiff = kernelDiff + userDiff;
    if (totalDiff == 0) return 0.0;
    return (double)(totalDiff - idleDiff) * 100.0 / totalDiff;
}

// --- Global State ---
std::atomic<bool> g_Monitoring = false;
novadesk::JsFunction* g_OnCpuUpdate = nullptr;
novadesk::Dispatcher* g_Dispatcher = nullptr;
const NovadeskHostAPI* g_Host = nullptr;

void OnCpuUpdateMainThread(void* data) {
    double usage = *(double*)data;
    delete (double*)data;

    if (g_OnCpuUpdate && g_OnCpuUpdate->IsValid()) {
        g_OnCpuUpdate->Call(usage);
    }
}

// --- Addon Entry Point ---
NOVADESK_ADDON_INIT(ctx, hMsgWnd, host) {
    g_Host = host;
    novadesk::Addon addon(ctx, host);
    g_Dispatcher = new novadesk::Dispatcher(hMsgWnd);

    // Initialize CPU times
    GetSystemTimes(&g_prevIdleTime, &g_prevKernelTime, &g_prevUserTime);

    // Register monitoring function
    addon.RegisterFunction("start", [](novadesk_context ctx) -> int {
        novadesk::Addon addon(ctx, g_Host);
        if (!addon.IsFunction(0)) {
            addon.ThrowError("start() requires a callback function");
            return 0;
        }

        if (g_OnCpuUpdate) delete g_OnCpuUpdate;
        g_OnCpuUpdate = new novadesk::JsFunction(ctx, g_Host, 0);

        if (!g_Monitoring) {
            g_Monitoring = true;
            std::thread([]() {
                while (g_Monitoring) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    if (g_Monitoring && g_Dispatcher) {
                        double* usage = new double(GetCPUUsage());
                        g_Dispatcher->Dispatch(OnCpuUpdateMainThread, usage);
                    }
                }
            }).detach();
        }
        return 0;
    }, 1);

    addon.RegisterFunction("stop", [](novadesk_context ctx) -> int {
        g_Monitoring = false;
        return 0;
    }, 0);
}

NOVADESK_ADDON_UNLOAD() {
    g_Monitoring = false;
    if (g_OnCpuUpdate) { delete g_OnCpuUpdate; g_OnCpuUpdate = nullptr; }
    if (g_Dispatcher) { delete g_Dispatcher; g_Dispatcher = nullptr; }
}
