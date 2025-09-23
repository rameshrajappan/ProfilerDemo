//====================================================
// File: dllmain.hpp
//====================================================
#pragma once
#include "stdafx.h"
#include "ProfilerClassFactory.hpp"

static HMODULE g_hModule = nullptr;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hModule = (HMODULE)hInst;
        DisableThreadLibraryCalls((HMODULE)hInst);
    }
    return TRUE;
}

extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;
    if (rclsid == CLSID_Profiler) {
        ProfilerClassFactory* f = new (std::nothrow) ProfilerClassFactory();
        if (!f) return E_OUTOFMEMORY;
        HRESULT hr = f->QueryInterface(riid, ppv);
        f->Release();
        return hr;
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

extern "C" HRESULT __stdcall DllCanUnloadNow() {
    return S_OK; // no global ref counting
}

// ---- OPTIONAL .def (if you prefer exporting via module definition) ----
// LIBRARY "YourProfiler"
// EXPORTS
//     DllGetClassObject
//     DllCanUnloadNow
