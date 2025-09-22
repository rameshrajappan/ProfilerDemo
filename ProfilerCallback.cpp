#include <initguid.h> // ensures CLSID_Profiler gets a definition here
#include "ProfilerCallback.h"
#include "Logger.h"

#define SAFE_RELEASE(p) do { if ((p) != nullptr) { (p)->Release(); (p) = nullptr; } } while(0)

// --- Replace this CLSID with YOUR OWN (Tools -> Create GUID -> Registry format) ---
const CLSID CLSID_Profiler =
{ 0xb1a7a0b9, 0x7f1a, 0x4e14, { 0x8d, 0x5e, 0x8a, 0x3a, 0xf7, 0xd2, 0xd5, 0xcb } };
// -------------------------------------------------------------------------------

ProfilerCallback::ProfilerCallback() {}
ProfilerCallback::~ProfilerCallback() {
    SAFE_RELEASE(m_info4);
}

// IUnknown
HRESULT STDMETHODCALLTYPE ProfilerCallback::QueryInterface(REFIID riid, void** ppInterface) 
{

    // Get interface from riid
    if (riid == IID_IUnknown)
        *ppInterface = static_cast<IUnknown*>(this);   
    else if (riid == __uuidof(ICorProfilerCallback5))
        *ppInterface = static_cast<ICorProfilerCallback5*>(this);
    else if (riid == __uuidof(ICorProfilerCallback4))
        *ppInterface = static_cast<ICorProfilerCallback4*>(this);
    else if (riid == __uuidof(ICorProfilerCallback3))
        *ppInterface = static_cast<ICorProfilerCallback3*>(this);
    else if (riid == __uuidof(ICorProfilerCallback2))
        *ppInterface = static_cast<ICorProfilerCallback2*>(this);
    else if (riid == __uuidof(ICorProfilerCallback))
        *ppInterface = static_cast<ICorProfilerCallback*>(this);
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    // Interface was successfully inferred, increase its reference count.
    reinterpret_cast<IUnknown*>(*ppInterface)->AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE ProfilerCallback::AddRef() { return (ULONG)++m_refCount; }
ULONG STDMETHODCALLTYPE ProfilerCallback::Release() {
    ULONG c = (ULONG)--m_refCount;
    if (c == 0) delete this;
    return c;
}

// ICorProfilerCallback
HRESULT STDMETHODCALLTYPE ProfilerCallback::Initialize(IUnknown* pICorProfilerInfoUnk) {
    if (!pICorProfilerInfoUnk) return E_POINTER;

    g_log.InitOnce();
    DWORD pid = GetCurrentProcessId();
    g_log.WriteLine(L"Initialize(): S_OK, PID=%lu", pid);
    (void)pICorProfilerInfoUnk->QueryInterface(__uuidof(ICorProfilerInfo4), (void**)&m_info4);

    DWORD eventMaskLow =
        COR_PRF_MONITOR_MODULE_LOADS |
        COR_PRF_MONITOR_JIT_COMPILATION ;


    if (m_info4) {
        HRESULT hr = m_info4->SetEventMask(eventMaskLow);
        if (SUCCEEDED(hr)) return S_OK;
    }

    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE ProfilerCallback::Shutdown() {
    SAFE_RELEASE(m_info4);
    return S_OK;
}

// --- No-op stubs (return S_OK). Fill these as needed. ---
HRESULT STDMETHODCALLTYPE ProfilerCallback::AppDomainCreationStarted(AppDomainID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::AppDomainCreationFinished(AppDomainID, HRESULT) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::AppDomainShutdownStarted(AppDomainID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::AppDomainShutdownFinished(AppDomainID, HRESULT) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::AssemblyLoadStarted(AssemblyID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::AssemblyLoadFinished(AssemblyID, HRESULT) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::AssemblyUnloadStarted(AssemblyID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::AssemblyUnloadFinished(AssemblyID, HRESULT) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ModuleLoadStarted(ModuleID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
{ 
    g_log.WriteLine(L"ModuleLoadFinished(): moduleId=%p, hr=0x%08X", (void*)moduleId, hrStatus);
    return S_OK; 
}
HRESULT STDMETHODCALLTYPE ProfilerCallback::ModuleUnloadStarted(ModuleID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ModuleUnloadFinished(ModuleID, HRESULT) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ModuleAttachedToAssembly(ModuleID, AssemblyID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ClassLoadStarted(ClassID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ClassLoadFinished(ClassID, HRESULT) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ClassUnloadStarted(ClassID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ClassUnloadFinished(ClassID, HRESULT) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::FunctionUnloadStarted(FunctionID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::JITCompilationStarted(FunctionID functionId, BOOL)
{ 
    long count = g_jitLoggedCount.fetch_add(1, std::memory_order_relaxed);
    if (count < kMaxJitLogs) {
        g_log.WriteLine(L"JITCompilationStarted(): functionId=%p (log %ld/%ld)",
            (void*)functionId, count + 1, kMaxJitLogs);
    }
    else if (count == kMaxJitLogs) {
        g_log.WriteLine(L"JITCompilationStarted(): reached log cap (%ld), suppressing further logs", kMaxJitLogs);
    }

    return S_OK; 
}
HRESULT STDMETHODCALLTYPE ProfilerCallback::JITCompilationFinished(FunctionID, HRESULT, BOOL) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::JITCachedFunctionSearchStarted(FunctionID, BOOL*) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::JITCachedFunctionSearchFinished(FunctionID, COR_PRF_JIT_CACHE) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::JITFunctionPitched(FunctionID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::JITInlining(FunctionID, FunctionID, BOOL*) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ThreadCreated(ThreadID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ThreadDestroyed(ThreadID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ThreadAssignedToOSThread(ThreadID, DWORD) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RemotingClientInvocationStarted() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RemotingClientSendingMessage(GUID*, BOOL) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RemotingClientReceivingReply(GUID*, BOOL) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RemotingClientInvocationFinished() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RemotingServerReceivingMessage(GUID*, BOOL) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RemotingServerInvocationStarted() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RemotingServerInvocationReturned() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RemotingServerSendingReply(GUID*, BOOL) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::UnmanagedToManagedTransition(FunctionID, COR_PRF_TRANSITION_REASON) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ManagedToUnmanagedTransition(FunctionID, COR_PRF_TRANSITION_REASON) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RuntimeSuspendFinished() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RuntimeSuspendAborted() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RuntimeResumeStarted() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RuntimeResumeFinished() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RuntimeThreadSuspended(ThreadID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RuntimeThreadResumed(ThreadID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::MovedReferences(ULONG, ObjectID[], ObjectID[], ULONG[]) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ObjectAllocated(ObjectID, ClassID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ObjectsAllocatedByClass(ULONG, ClassID[], ULONG[]) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ObjectReferences(ObjectID, ClassID, ULONG, ObjectID[]) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RootReferences(ULONG, ObjectID[]) { return S_OK; }

HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionThrown(ObjectID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionSearchFunctionEnter(FunctionID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionSearchFunctionLeave() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionSearchFilterEnter(FunctionID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionSearchFilterLeave() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionSearchCatcherFound(FunctionID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionCLRCatcherFound() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionCLRCatcherExecute() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionOSHandlerEnter(UINT_PTR) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionOSHandlerLeave(UINT_PTR) { return S_OK; }

HRESULT STDMETHODCALLTYPE ProfilerCallback::COMClassicVTableCreated(ClassID, REFGUID, void*, ULONG) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::COMClassicVTableDestroyed(ClassID, REFGUID, void*) { return S_OK; }

HRESULT STDMETHODCALLTYPE ProfilerCallback::ThreadNameChanged(ThreadID, ULONG, WCHAR[]) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::GarbageCollectionStarted(int, BOOL[], COR_PRF_GC_REASON) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::SurvivingReferences(ULONG, ObjectID[], ULONG[]) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::GarbageCollectionFinished() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::FinalizeableObjectQueued(DWORD, ObjectID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::RootReferences2(ULONG, ObjectID[], COR_PRF_GC_ROOT_KIND[], COR_PRF_GC_ROOT_FLAGS[], UINT_PTR[]) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::HandleCreated(GCHandleID, ObjectID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::HandleDestroyed(GCHandleID) { return S_OK; }

HRESULT STDMETHODCALLTYPE ProfilerCallback::InitializeForAttach(IUnknown*, void*, UINT) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ProfilerAttachComplete() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ProfilerDetachSucceeded() { return S_OK; }

HRESULT STDMETHODCALLTYPE ProfilerCallback::ReJITCompilationStarted(FunctionID, ReJITID, BOOL) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::GetReJITParameters(
    ModuleID, mdMethodDef, ICorProfilerFunctionControl* ctrl) {
    if (ctrl) ctrl->SetILInstrumentedCodeMap(0, nullptr); // FIXED
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ProfilerCallback::ReJITCompilationFinished(FunctionID, ReJITID, HRESULT, BOOL) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ReJITError(ModuleID, mdMethodDef, FunctionID, HRESULT) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::MovedReferences2(ULONG, ObjectID[], ObjectID[], SIZE_T[]) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::SurvivingReferences2(ULONG, ObjectID[], SIZE_T[]) { return S_OK; }

HRESULT STDMETHODCALLTYPE ProfilerCallback::ConditionalWeakTableElementReferences(ULONG, ObjectID[], ObjectID[], GCHandleID[]) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionUnwindFunctionEnter(FunctionID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionUnwindFunctionLeave() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionUnwindFinallyEnter(FunctionID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionUnwindFinallyLeave() { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionCatcherEnter(FunctionID, ObjectID) { return S_OK; }
HRESULT STDMETHODCALLTYPE ProfilerCallback::ExceptionCatcherLeave() { return S_OK; }
