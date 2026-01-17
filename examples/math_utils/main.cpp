#include <NovadeskAPI/novadesk_addon.h>

const NovadeskHostAPI* g_Host = nullptr;

/** Math Addon Entry Point */
NOVADESK_ADDON_INIT(ctx, hMsgWnd, host) {
    g_Host = host;
    novadesk::Addon addon(ctx, host);

    // Register simple math functions
    addon.RegisterFunction("sum", [](novadesk_context ctx) -> int {
        novadesk::Addon addon(ctx, g_Host);
        if (!addon.IsNumber(0) || !addon.IsNumber(1)) {
            addon.ThrowError("sum() requires two numeric arguments");
            return 0;
        }
        double a = addon.GetNumber(0);
        double b = addon.GetNumber(1);
        g_Host->PushNumber(ctx, a + b);
        return 1;
    }, 2);

    addon.RegisterFunction("subtract", [](novadesk_context ctx) -> int {
        novadesk::Addon addon(ctx, g_Host);
        if (!addon.IsNumber(0) || !addon.IsNumber(1)) {
            addon.ThrowError("subtract() requires two numeric arguments");
            return 0;
        }
        double a = addon.GetNumber(0);
        double b = addon.GetNumber(1);
        g_Host->PushNumber(ctx, a - b);
        return 1;
    }, 2);

    addon.RegisterFunction("multiply", [](novadesk_context ctx) -> int {
        novadesk::Addon addon(ctx, g_Host);
        if (!addon.IsNumber(0) || !addon.IsNumber(1)) {
            addon.ThrowError("multiply() requires two numeric arguments");
            return 0;
        }
        double a = addon.GetNumber(0);
        double b = addon.GetNumber(1);
        g_Host->PushNumber(ctx, a * b);
        return 1;
    }, 2);
}

NOVADESK_ADDON_UNLOAD() {
    // No cleanup needed for stateless math functions
}
