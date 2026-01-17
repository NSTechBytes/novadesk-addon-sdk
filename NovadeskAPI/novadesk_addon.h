/* Copyright (C) 2026 Novadesk Project 
 *
 * This Source Code Form is subject to the terms of the GNU General Public
 * License; either version 2 of the License, or (at your option) any later
 * version. If a copy of the GPL was not distributed with this file, You can
 * obtain one at <https://www.gnu.org/licenses/gpl-2.0.html>. */

#pragma once

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque context handle (hides Duktape context)
typedef void* novadesk_context;

// Host API interface provided by Novadesk
struct NovadeskHostAPI {
    void (*RegisterString)(novadesk_context ctx, const char* name, const char* value);
    void (*RegisterNumber)(novadesk_context ctx, const char* name, double value);
    void (*RegisterBool)(novadesk_context ctx, const char* name, int value);
    void (*RegisterObjectStart)(novadesk_context ctx, const char* name);
    void (*RegisterObjectEnd)(novadesk_context ctx, const char* name);
    void (*RegisterArrayString)(novadesk_context ctx, const char* name, const char** values, size_t count);
    void (*RegisterArrayNumber)(novadesk_context ctx, const char* name, const double* values, size_t count);
    void (*RegisterFunction)(novadesk_context ctx, const char* name, int (*func)(novadesk_context ctx), int nargs);
    void (*PushString)(novadesk_context ctx, const char* value);
    void (*PushNumber)(novadesk_context ctx, double value);
    void (*PushBool)(novadesk_context ctx, int value);
    void (*PushNull)(novadesk_context ctx);
    void (*PushObject)(novadesk_context ctx);
    void* (*JsGetFunctionPtr)(novadesk_context ctx, int index);
    void (*JsCallFunction)(novadesk_context ctx, void* funcPtr, int nargs);
};

// Define the initialization function signature
typedef void (*NovadeskAddonInitFn)(novadesk_context ctx, HWND hMsgWnd, const NovadeskHostAPI* host);

// Define the cleanup function signature (optional)
typedef void (*NovadeskAddonUnloadFn)();

// Entry Point Macros
#define NOVADESK_ADDON_INIT(ctx, hMsgWnd, host) extern "C" __declspec(dllexport) void NovadeskAddonInit(novadesk_context ctx, HWND hMsgWnd, const NovadeskHostAPI* host)
#define NOVADESK_ADDON_UNLOAD() extern "C" __declspec(dllexport) void NovadeskAddonUnload()

#ifdef __cplusplus
}

#include <vector>
#include <string>

// C++ Helper Utilities
namespace novadesk {
    class JsFunction {
    public:
        JsFunction(novadesk_context ctx, const NovadeskHostAPI* host, int idx) : m_ctx(ctx), m_host(host) {
            m_ptr = m_host->JsGetFunctionPtr(m_ctx, idx);
        }

        bool IsValid() const { return m_ptr != nullptr; }

        void Call() {
            if (!IsValid()) return;
            m_host->JsCallFunction(m_ctx, m_ptr, 0);
        }

        void Call(const char* arg) {
            if (!IsValid()) return;
            m_host->PushString(m_ctx, arg);
            m_host->JsCallFunction(m_ctx, m_ptr, 1);
        }

        void Call(double arg) {
            if (!IsValid()) return;
            m_host->PushNumber(m_ctx, arg);
            m_host->JsCallFunction(m_ctx, m_ptr, 1);
        }

    private:
        novadesk_context m_ctx;
        const NovadeskHostAPI* m_host;
        void* m_ptr;
    };

    class Dispatcher {
    public:
        Dispatcher(HWND hMsgWnd) : m_hWnd(hMsgWnd) {}

        void Dispatch(void (*fn)(void*), void* data = nullptr) {
            if (m_hWnd) {
                PostMessage(m_hWnd, WM_NOVADESK_DISPATCH, (WPARAM)fn, (LPARAM)data);
            }
        }

    private:
        HWND m_hWnd;
        static const UINT WM_NOVADESK_DISPATCH = WM_USER + 101;
    };

    class Addon {
    public:
        Addon(novadesk_context ctx, const NovadeskHostAPI* host) : m_ctx(ctx), m_host(host) {
            m_host->PushObject(m_ctx);
        }

        template<typename F>
        void RegisterObject(const char* name, F populateFunc) {
            m_host->RegisterObjectStart(m_ctx, name);
            Addon sub(m_ctx, m_host, true);
            populateFunc(sub);
            m_host->RegisterObjectEnd(m_ctx, name);
        }

        void RegisterFunction(const char* name, int (*func)(novadesk_context ctx), int nargs = 0) {
            m_host->RegisterFunction(m_ctx, name, func, nargs);
        }

        void RegisterStringFunction(const char* name, const char* value) {
            // Helper for simple static strings
            RegisterFunction(name, [](novadesk_context ctx) -> int {
                // This is a bit tricky without a data capture in the host API...
                // For now, let's skip this or implement a more complex registration.
                return 0; 
            });
        }

        void RegisterString(const char* name, const char* value) {
            m_host->RegisterString(m_ctx, name, value);
        }

        void RegisterNumber(const char* name, double value) {
            m_host->RegisterNumber(m_ctx, name, value);
        }

        void RegisterBool(const char* name, bool value) {
            m_host->RegisterBool(m_ctx, name, value ? 1 : 0);
        }

        void RegisterArray(const char* name, const std::vector<std::string>& values) {
            std::vector<const char*> ptrs;
            for (const auto& s : values) ptrs.push_back(s.c_str());
            m_host->RegisterArrayString(m_ctx, name, ptrs.data(), ptrs.size());
        }

        void RegisterArray(const char* name, const std::vector<double>& values) {
            m_host->RegisterArrayNumber(m_ctx, name, values.data(), values.size());
        }

    private:
        Addon(novadesk_context ctx, const NovadeskHostAPI* host, bool) : m_ctx(ctx), m_host(host) {}

        novadesk_context m_ctx;
        const NovadeskHostAPI* m_host;
    };
}
#endif
