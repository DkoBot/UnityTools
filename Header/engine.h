#pragma once
#include "base.h"
#include "class.h"
#include "../Misc/kiero/kiero.h"
#include <windows.h>
#include <dwmapi.h>

typedef struct Il2CppType Il2CppType;
typedef struct EventInfo EventInfo;

// MethodInfo 结构体定义（IL2CPP中的基本结构）
typedef struct MethodInfo {
    const char* name;
    // 其他成员暂时不需要
} MethodInfo;

// FieldInfo 结构体定义（IL2CPP中的基本结构）
typedef struct FieldInfo {
    const char* name;
    const Il2CppType* type;
    // 其他成员暂时不需要
} FieldInfo;
typedef struct PropertyInfo PropertyInfo;

typedef struct Il2CppAssembly Il2CppAssembly;
typedef struct Il2CppArray Il2CppArray;
typedef struct Il2CppDelegate Il2CppDelegate;
typedef struct Il2CppDomain Il2CppDomain;
typedef struct Il2CppImage Il2CppImage;
typedef struct Il2CppException Il2CppException;
typedef struct Il2CppProfiler Il2CppProfiler;
typedef struct Il2CppObject Il2CppObject;
typedef struct Il2CppReflectionMethod Il2CppReflectionMethod;
typedef struct Il2CppReflectionType Il2CppReflectionType;
typedef struct Il2CppString Il2CppString;
typedef struct Il2CppThread Il2CppThread;
typedef struct Il2CppAsyncResult Il2CppAsyncResult;
typedef struct Il2CppManagedMemorySnapshot Il2CppManagedMemorySnapshot;
typedef struct Il2CppCustomAttrInfo Il2CppCustomAttrInfo;

typedef LRESULT(WINAPI* pWndProc)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(WINAPI* pPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(WINAPI* pResize)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

#define DO_API(RetType,Name,Args) \
using Name##_t = RetType(__stdcall*)Args; \
extern Name##_t Name;
#include "il2cpp_api.h"
#undef DO_API

#define DO_FUNC(RetType,Name,Args,AssemblyName,Namespaze,ClassName,Func) \
using Name##_t = RetType(__stdcall*)Args; \
extern Name##_t Name;
#include "il2cpp_function.h"
#undef DO_FUNC



namespace Engine {
	extern HWND hWnd;
	extern HMODULE hUnityPlayer;
	extern HMODULE hGameAssembly;

	string il2cppStringToStdString(Il2CppString* il2cppStr);
	void initialize();
	HWND GetHwnd();
	void* GetMethod(string AssemblyName, string Namespaze, string ClassName, string Func);
	void* GetMethodByName(string AssemblyName, string Namespaze, string ClassName, string MethodName, int paramCount);
	Il2CppString* create_il2cpp_string(const wchar_t* src);
}