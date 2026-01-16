#include <NovadeskAPI/novadesk_addon.h>

// The initialization function is called when the addon is loaded.
NOVADESK_ADDON_INIT(ctx) {
    // Use the C++ helper to manage registration
    novadesk::Addon addon(ctx);

    // Register properties and functions easily
    addon.RegisterStringFunction("hello", "Hello from the abstracted Addon SDK!");

    addon.RegisterString("version", "1.0.0");
    addon.RegisterNumber("id", 42);
    addon.RegisterBool("isNative", true);
}

// Optional: The unload function is called when the script reloads or Novadesk exits.
NOVADESK_ADDON_UNLOAD() {
    // Perform any necessary cleanup here (no Duktape context available here)
}
