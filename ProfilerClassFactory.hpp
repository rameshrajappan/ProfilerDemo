#pragma once
#include <unknwn.h>
#include <atomic>
#include "ProfilerCallback.h"

class ProfilerClassFactory final : public IClassFactory {
public:
    ProfilerClassFactory() : m_ref(1) {}

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IClassFactory)) {
            *ppv = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return (ULONG)++m_ref; }
    ULONG STDMETHODCALLTYPE Release() override { ULONG c = (ULONG)--m_ref; if (!c) delete this; return c; }

    // IClassFactory
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        if (pUnkOuter) return CLASS_E_NOAGGREGATION;
        ProfilerCallback* p = new (std::nothrow) ProfilerCallback();
        if (!p) return E_OUTOFMEMORY;
        HRESULT hr = p->QueryInterface(riid, ppv);
        p->Release(); // balance initial ref
        return hr;
    }
    HRESULT STDMETHODCALLTYPE LockServer(BOOL) override { return S_OK; }

private:
    std::atomic<ULONG> m_ref;
};
