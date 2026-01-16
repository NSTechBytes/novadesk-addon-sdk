# Novadesk Addon SDK

Official SDK for developing external addons for Novadesk.

## Key Features

- **Dynamic Loading**: Use `system.loadAddon("path/to/addon.dll")` to load modules at runtime.
- **Lifecycle Hooks**: 
    - `NovadeskAddonInit`: Called when the addon is loaded. Use this to register your API.
    - `NovadeskAddonUnload`: Called when the script reloads or Novadesk exits. Use this to clean up resources.
- **Shared Header**: A standard [novadesk_addon.h](file:///d:/GITHUB/novadesk-addon-sdk/include/novadesk_addon.h) provides the interface.
- **Visual Studio Integration**: Includes a property sheet (`NovadeskAddon.props`) for easy project configuration.

## Getting Started

1.  **Clone the SDK**: `git clone https://github.com/novadesk/novadesk-addon-sdk`
2.  **Create a New Project**:
    - Create a "C++ Desktop DLL" project in Visual Studio.
    - Add [NovadeskAddon.props](file:///d:/GITHUB/novadesk-addon-sdk/NovadeskAddon.props) to your project via the **Property Manager**.
    - This will automatically configure include paths and output directories.
3.  **Implement the Interface**:
    - Include `novadesk_addon.h`.
    - Add `include/duktape/duktape.c` to your project to provide the Duktape implementation.
    - Implement the exported `NovadeskAddonInit`.
4.  **Build**: Build as `Win32` (x86).

## Project Structure

- `include/`: SDK headers (`novadesk_addon.h` and `duktape/`).
- `examples/`: Sample addon projects.
- `NovadeskAddon.props`: Visual Studio property sheet.
- `NovadeskAddons.sln`: Solution for all examples.

## Example (C++)

```cpp
#include "novadesk_addon.h"

extern "C" __declspec(dllexport) void NovadeskAddonInit(duk_context* ctx) {
    // Register your functions here
    duk_push_object(ctx);
    duk_push_string(ctx, "Hello from C++!");
    duk_put_prop_string(ctx, -2, "message");
    // The object at the top of the stack is returned to JS
}
```

## Loading Addons

In your Novadesk JavaScript:

```javascript
const addon = system.loadAddon("./MyAddon.dll");
console.log(addon); // "Hello from Addon"
```
