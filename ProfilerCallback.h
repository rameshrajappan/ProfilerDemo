#pragma once
#define _WIN32_DCOM
#include <windows.h>
#include <unknwn.h>
#include <atomic>
#include <cor.h>
#include <corprof.h>

#ifndef COM_METHOD
#  define COM_METHOD(TYPE) virtual TYPE STDMETHODCALLTYPE
#endif

// Extern CLSID declaration (defined in ProfilerCallback.cpp)
extern const CLSID CLSID_Profiler;

class ProfilerCallback final : public ICorProfilerCallback5 {
public:
    ProfilerCallback();
    virtual ~ProfilerCallback();

    // IUnknown
    COM_METHOD(HRESULT) QueryInterface(REFIID riid, void** ppv) override;
    COM_METHOD(ULONG)   AddRef() override;
    COM_METHOD(ULONG)   Release() override;

    // ICorProfilerCallback
    COM_METHOD(HRESULT) Initialize(IUnknown* pICorProfilerInfoUnk) override;
    COM_METHOD(HRESULT) Shutdown() override;

    COM_METHOD(HRESULT) AppDomainCreationStarted(AppDomainID appDomainId) override;
    COM_METHOD(HRESULT) AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus) override;
    COM_METHOD(HRESULT) AppDomainShutdownStarted(AppDomainID appDomainId) override;
    COM_METHOD(HRESULT) AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus) override;

    COM_METHOD(HRESULT) AssemblyLoadStarted(AssemblyID assemblyId) override;
    COM_METHOD(HRESULT) AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus) override;
    COM_METHOD(HRESULT) AssemblyUnloadStarted(AssemblyID assemblyId) override;
    COM_METHOD(HRESULT) AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus) override;

    COM_METHOD(HRESULT) ModuleLoadStarted(ModuleID moduleId) override;
    COM_METHOD(HRESULT) ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus) override;
    COM_METHOD(HRESULT) ModuleUnloadStarted(ModuleID moduleId) override;
    COM_METHOD(HRESULT) ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus) override;
    COM_METHOD(HRESULT) ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID assemblyId) override;

    COM_METHOD(HRESULT) ClassLoadStarted(ClassID classId) override;
    COM_METHOD(HRESULT) ClassLoadFinished(ClassID classId, HRESULT hrStatus) override;
    COM_METHOD(HRESULT) ClassUnloadStarted(ClassID classId) override;
    COM_METHOD(HRESULT) ClassUnloadFinished(ClassID classId, HRESULT hrStatus) override;

    COM_METHOD(HRESULT) FunctionUnloadStarted(FunctionID functionId) override;

    COM_METHOD(HRESULT) JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock) override;
    COM_METHOD(HRESULT) JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock) override;
    COM_METHOD(HRESULT) JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction) override;
    COM_METHOD(HRESULT) JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result) override;
    COM_METHOD(HRESULT) JITFunctionPitched(FunctionID functionId) override;
    COM_METHOD(HRESULT) JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline) override;

    COM_METHOD(HRESULT) ThreadCreated(ThreadID threadId) override;
    COM_METHOD(HRESULT) ThreadDestroyed(ThreadID threadId) override;
    COM_METHOD(HRESULT) ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId) override;

    COM_METHOD(HRESULT) RemotingClientInvocationStarted() override;
    COM_METHOD(HRESULT) RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync) override;
    COM_METHOD(HRESULT) RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync) override;
    COM_METHOD(HRESULT) RemotingClientInvocationFinished() override;
    COM_METHOD(HRESULT) RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync) override;
    COM_METHOD(HRESULT) RemotingServerInvocationStarted() override;
    COM_METHOD(HRESULT) RemotingServerInvocationReturned() override;
    COM_METHOD(HRESULT) RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync) override;

    COM_METHOD(HRESULT) UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason) override;
    COM_METHOD(HRESULT) ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason) override;

    COM_METHOD(HRESULT) RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason) override;
    COM_METHOD(HRESULT) RuntimeSuspendFinished() override;
    COM_METHOD(HRESULT) RuntimeSuspendAborted() override;
    COM_METHOD(HRESULT) RuntimeResumeStarted() override;
    COM_METHOD(HRESULT) RuntimeResumeFinished() override;
    COM_METHOD(HRESULT) RuntimeThreadSuspended(ThreadID threadId) override;
    COM_METHOD(HRESULT) RuntimeThreadResumed(ThreadID threadId) override;

    COM_METHOD(HRESULT) MovedReferences(ULONG cmovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[]) override;
    COM_METHOD(HRESULT) ObjectAllocated(ObjectID objectId, ClassID classId) override;
    COM_METHOD(HRESULT) ObjectsAllocatedByClass(ULONG classCount, ClassID classIds[], ULONG cObjects[]) override;
    COM_METHOD(HRESULT) ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[]) override;
    COM_METHOD(HRESULT) RootReferences(ULONG cRootRefs, ObjectID rootRefIds[]) override;

    // Exceptions (OS handler uses UINT_PTR per corprof.h)
    COM_METHOD(HRESULT) ExceptionThrown(ObjectID thrownObjectID) override;
    COM_METHOD(HRESULT) ExceptionSearchFunctionEnter(FunctionID functionID) override;
    COM_METHOD(HRESULT) ExceptionSearchFunctionLeave() override;
    COM_METHOD(HRESULT) ExceptionSearchFilterEnter(FunctionID functionID) override;
    COM_METHOD(HRESULT) ExceptionSearchFilterLeave() override;
    COM_METHOD(HRESULT) ExceptionSearchCatcherFound(FunctionID functionID) override;
    COM_METHOD(HRESULT) ExceptionCLRCatcherFound() override;
    COM_METHOD(HRESULT) ExceptionCLRCatcherExecute() override;
    COM_METHOD(HRESULT) ExceptionOSHandlerEnter(UINT_PTR) override;
    COM_METHOD(HRESULT) ExceptionOSHandlerLeave(UINT_PTR) override;

    COM_METHOD(HRESULT) COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots) override;
    COM_METHOD(HRESULT) COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable) override;

    // ICorProfilerCallback2
    COM_METHOD(HRESULT) ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[]) override;
    COM_METHOD(HRESULT) GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason) override;
    COM_METHOD(HRESULT) SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[]) override;
    COM_METHOD(HRESULT) GarbageCollectionFinished() override;
    COM_METHOD(HRESULT) FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID) override;
    COM_METHOD(HRESULT) RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[]) override;
    COM_METHOD(HRESULT) HandleCreated(GCHandleID handleId, ObjectID initialObjectId) override;
    COM_METHOD(HRESULT) HandleDestroyed(GCHandleID handleId) override;

    // ICorProfilerCallback3
    COM_METHOD(HRESULT) InitializeForAttach(IUnknown* pCorProfilerInfoUnk, void* pvClientData, UINT cbClientData) override;
    COM_METHOD(HRESULT) ProfilerAttachComplete() override;
    COM_METHOD(HRESULT) ProfilerDetachSucceeded() override;

    // ICorProfilerCallback4
    COM_METHOD(HRESULT) ReJITCompilationStarted(FunctionID functionId, ReJITID rejitId, BOOL fIsSafeToBlock) override;
    COM_METHOD(HRESULT) GetReJITParameters(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl) override;
    COM_METHOD(HRESULT) ReJITCompilationFinished(FunctionID functionId, ReJITID rejitId, HRESULT hrStatus, BOOL fIsSafeToBlock) override;
    COM_METHOD(HRESULT) ReJITError(ModuleID moduleId, mdMethodDef methodId, FunctionID functionId, HRESULT hrStatus) override;
    COM_METHOD(HRESULT) MovedReferences2(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], SIZE_T cObjectIDRangeLength[]) override;
    COM_METHOD(HRESULT) SurvivingReferences2(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], SIZE_T cObjectIDRangeLength[]) override;

    // ICorProfilerCallback5
    COM_METHOD(HRESULT) ConditionalWeakTableElementReferences(ULONG cRootRefs, ObjectID keyRefIds[], ObjectID valueRefIds[], GCHandleID rootIds[]) override;

    COM_METHOD(HRESULT) ExceptionUnwindFunctionEnter(FunctionID functionID) override;
    COM_METHOD(HRESULT) ExceptionUnwindFunctionLeave() override;
    COM_METHOD(HRESULT) ExceptionUnwindFinallyEnter(FunctionID functionID) override;
    COM_METHOD(HRESULT) ExceptionUnwindFinallyLeave() override;
    COM_METHOD(HRESULT) ExceptionCatcherEnter(FunctionID functionID, ObjectID objectID) override;
    COM_METHOD(HRESULT) ExceptionCatcherLeave() override;


private:
    std::atomic<ULONG>  m_refCount{ 1 };
    ICorProfilerInfo4* m_info4{ nullptr };
};
