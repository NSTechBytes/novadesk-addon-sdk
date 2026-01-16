#include "novadesk_addon.h"
#include <iostream>

// The initialization function is called when the addon is loaded.
// You can use the duk_context to register global functions/objects or return a value to JS.
extern "C" __declspec(dllexport) void NovadeskAddonInit(duk_context* ctx) {
    // Create an object to return to JavaScript
    duk_push_object(ctx);

    // Add a hello function
    duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {
        duk_push_string(ctx, "Hello from the External Addon SDK!");
        return 1;
    }, 0);
    duk_put_prop_string(ctx, -2, "hello");

    // Add a version string
    duk_push_string(ctx, "1.0.0");
    duk_put_prop_string(ctx, -2, "version");

    // The object at the top of the stack will be returned by system.loadAddon()
}

// Optional: The unload function is called when the script reloads or Novadesk exits.
extern "C" __declspec(dllexport) void NovadeskAddonUnload() {
    // Perform any necessary cleanup here
    // Note: Do not use the duk_context here as it may already be partially destroyed.
}
