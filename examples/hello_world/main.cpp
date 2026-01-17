#include <NovadeskAPI/novadesk_addon.h>
#include <thread>
#include <atomic>

std::atomic<bool> g_Running = false;
novadesk::JsFunction* g_Callback = nullptr;
novadesk::Dispatcher* g_Dispatcher = nullptr;
const NovadeskHostAPI* g_Host = nullptr;

// This function runs on the MAIN THREAD
void OnPulse(void* data) {
    if (g_Callback && g_Callback->IsValid()) {
        g_Callback->Call("Pulse from Main Thread!");
    }
}

// The initialization function is called when the addon is loaded.
NOVADESK_ADDON_INIT(ctx, hMsgWnd, host) {
    g_Host = host;
    novadesk::Addon addon(ctx, host);
    g_Running = true;
    g_Dispatcher = new novadesk::Dispatcher(hMsgWnd);

    // Register properties and functions easily
    addon.RegisterString("version", "1.0.0");

    addon.RegisterFunction("hello", [](novadesk_context ctx) -> int {
        g_Host->PushString(ctx, "Hello from native C++ addon!");
        return 1;
    });

    // Nest objects for cleaner APIs
    addon.RegisterObject("utils", [](novadesk::Addon& utils) {
        utils.RegisterNumber("id", 123);
        
        // Return arrays of data
        utils.RegisterArray("tags", {"cpp", "native", "addon", "decoupled"});
        utils.RegisterArray("versions", {1.0, 1.1, 2.0});
    });

    // JavaScript Callbacks (Event Hooks)
    addon.RegisterFunction("onEvent", [](novadesk_context ctx) -> int {
        novadesk::JsFunction cb(ctx, g_Host, 0); // Capture the function at index 0
        if (cb.IsValid()) {
            cb.Call("Hello from C++ Callback!");
        }
        return 0;
    });

    // Persistent Background Thread
    addon.RegisterFunction("startPulse", [](novadesk_context ctx) -> int {
        if (g_Callback) delete g_Callback;
        g_Callback = new novadesk::JsFunction(ctx, g_Host, 1); // Fixed index 0 -> 1 because the callback is the first arg, but we need to account for 'this' if it were a method? No, duktape functions are 0-indexed args. Wait.
        // Actually, duktape args are 0, 1, 2...
        // Let's use index 0.
        g_Callback = new novadesk::JsFunction(ctx, g_Host, 0);

        if (!g_Running) {
            g_Running = true;
            std::thread([]() {
                while (g_Running) {
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    if (g_Running && g_Dispatcher) {
                        g_Dispatcher->Dispatch(OnPulse);
                    }
                }
            }).detach();
        }

        return 0;
    });
}

// Optional: The unload function is called when the script reloads or Novadesk exits.
NOVADESK_ADDON_UNLOAD() {
    g_Running = false;
    if (g_Callback) { delete g_Callback; g_Callback = nullptr; }
    if (g_Dispatcher) { delete g_Dispatcher; g_Dispatcher = nullptr; }
}
