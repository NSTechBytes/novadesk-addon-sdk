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

// Opaque context handle
typedef void* novadesk_context;

// Host API interface - Rich version
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

    double (*GetNumber)(novadesk_context ctx, int index);
    const char* (*GetString)(novadesk_context ctx, int index);
    int (*GetBool)(novadesk_context ctx, int index);

    int (*IsNumber)(novadesk_context ctx, int index);
    int (*IsString)(novadesk_context ctx, int index);
    int (*IsBool)(novadesk_context ctx, int index);
    int (*IsObject)(novadesk_context ctx, int index);
    int (*IsFunction)(novadesk_context ctx, int index);
    int (*IsNull)(novadesk_context ctx, int index);

    int (*GetTop)(novadesk_context ctx);
    void (*Pop)(novadesk_context ctx);
    void (*PopN)(novadesk_context ctx, int n);
    void (*ThrowError)(novadesk_context ctx, const char* message);

    void* (*JsGetFunctionPtr)(novadesk_context ctx, int index);
    void (*JsCallFunction)(novadesk_context ctx, void* funcPtr, int nargs);
};

// Initialization signature
typedef void (*NovadeskAddonInitFn)(novadesk_context ctx, HWND hMsgWnd, const NovadeskHostAPI* host);
typedef void (*NovadeskAddonUnloadFn)();

#define NOVADESK_ADDON_INIT(ctx, hMsgWnd, host) extern "C" __declspec(dllexport) void NovadeskAddonInit(novadesk_context ctx, HWND hMsgWnd, const NovadeskHostAPI* host)
#define NOVADESK_ADDON_UNLOAD() extern "C" __declspec(dllexport) void NovadeskAddonUnload()

#ifdef __cplusplus
}

#include <vector>
#include <string>

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
            if (m_hWnd) PostMessage(m_hWnd, WM_NOVADESK_DISPATCH, (WPARAM)fn, (LPARAM)data);
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

        void RegisterString(const char* name, const char* value) { m_host->RegisterString(m_ctx, name, value); }
        void RegisterNumber(const char* name, double value) { m_host->RegisterNumber(m_ctx, name, value); }
        void RegisterBool(const char* name, bool value) { m_host->RegisterBool(m_ctx, name, value ? 1 : 0); }

        void RegisterArray(const char* name, const std::vector<std::string>& values) {
            std::vector<const char*> ptrs;
            for (const auto& s : values) ptrs.push_back(s.c_str());
            m_host->RegisterArrayString(m_ctx, name, ptrs.data(), (size_t)ptrs.size());
        }

        void RegisterArray(const char* name, const std::vector<double>& values) {
            m_host->RegisterArrayNumber(m_ctx, name, values.data(), (size_t)values.size());
        }

        // --- Context Utilities ---
        int GetTop() { return m_host->GetTop(m_ctx); }
        void Pop() { m_host->Pop(m_ctx); }
        void PopN(int n) { m_host->PopN(m_ctx, n); }
        void ThrowError(const char* msg) { m_host->ThrowError(m_ctx, msg); }

        bool IsNumber(int idx) { return m_host->IsNumber(m_ctx, idx) != 0; }
        bool IsString(int idx) { return m_host->IsString(m_ctx, idx) != 0; }
        bool IsBool(int idx) { return m_host->IsBool(m_ctx, idx) != 0; }
        bool IsObject(int idx) { return m_host->IsObject(m_ctx, idx) != 0; }
        bool IsFunction(int idx) { return m_host->IsFunction(m_ctx, idx) != 0; }
        bool IsNull(int idx) { return m_host->IsNull(m_ctx, idx) != 0; }

        double GetNumber(int idx) { return m_host->GetNumber(m_ctx, idx); }
        const char* GetString(int idx) { return m_host->GetString(m_ctx, idx); }
        bool GetBool(int idx) { return m_host->GetBool(m_ctx, idx) != 0; }

    private:
        Addon(novadesk_context ctx, const NovadeskHostAPI* host, bool) : m_ctx(ctx), m_host(host) {}
        novadesk_context m_ctx;
        const NovadeskHostAPI* m_host;
    };
}
#endif
