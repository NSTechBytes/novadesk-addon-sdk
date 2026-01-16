/* Copyright (C) 2026 Novadesk Project 
 *
 * This Source Code Form is subject to the terms of the GNU General Public
 * License; either version 2 of the License, or (at your option) any later
 * version. If a copy of the GPL was not distributed with this file, You can
 * obtain one at <https://www.gnu.org/licenses/gpl-2.0.html>. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "duktape/duktape.h"

// Define the initialization function signature
typedef void (*NovadeskAddonInitFn)(duk_context* ctx);

// Define the cleanup function signature (optional)
typedef void (*NovadeskAddonUnloadFn)();

// Entry Point Macros
#define NOVADESK_ADDON_INIT(ctx) extern "C" __declspec(dllexport) void NovadeskAddonInit(duk_context* ctx)
#define NOVADESK_ADDON_UNLOAD() extern "C" __declspec(dllexport) void NovadeskAddonUnload()

#ifdef __cplusplus
}

// C++ Helper Utilities
namespace novadesk {
    class Addon {
    public:
        Addon(duk_context* ctx) : m_ctx(ctx) {
            duk_push_object(m_ctx);
        }

        // Register a C-style function
        void RegisterFunction(const char* name, duk_c_function func, duk_idx_t nargs = 0) {
            duk_push_c_function(m_ctx, func, nargs);
            duk_put_prop_string(m_ctx, -2, name);
        }

        // Register a function that returns a constant string
        void RegisterStringFunction(const char* name, const char* value) {
            duk_push_c_function(m_ctx, [](duk_context* ctx) -> duk_ret_t {
                duk_push_current_function(ctx);
                duk_get_prop_string(ctx, -1, "\xff" "data");
                return 1;
            }, 0);
            duk_push_string(m_ctx, value);
            duk_put_prop_string(m_ctx, -2, "\xff" "data");
            duk_put_prop_string(m_ctx, -2, name);
        }

        // Register a string property
        void RegisterString(const char* name, const char* value) {
            duk_push_string(m_ctx, value);
            duk_put_prop_string(m_ctx, -2, name);
        }

        // Register a number property
        void RegisterNumber(const char* name, double value) {
            duk_push_number(m_ctx, value);
            duk_put_prop_string(m_ctx, -2, name);
        }

        // Register a boolean property
        void RegisterBool(const char* name, bool value) {
            duk_push_boolean(m_ctx, value ? 1 : 0);
            duk_put_prop_string(m_ctx, -2, name);
        }

    private:
        duk_context* m_ctx;
    };
}
#endif
