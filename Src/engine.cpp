#include "engine.h"

#define DO_API(RetType,Name,Args)\
	Name##_t Name = NULL;
#include "il2cpp_api.h"
#undef DO_API

#define DO_FUNC(RetType,Name,Args,AssemblyName,Namespaze,ClassName,Func)\
	Name##_t Name = NULL;
#include "il2cpp_function.h"
#undef DO_FUNC

namespace Engine {
	HWND hWnd = NULL;
	HMODULE hUnityPlayer = NULL;
	HMODULE hGameAssembly = NULL;
	void initialize() {
		hWnd = FindWindowA("UnityWndClass", NULL);
		hUnityPlayer = GetModuleHandleA("UnityPlayer.dll");
		if (!hUnityPlayer) {
			MessageBox(NULL, L"Not Find UnityPlayer.dll", L"Init Error", MB_OK);
		}
		hGameAssembly = GetModuleHandleA("GameAssembly.dll");
		if (!hGameAssembly) {
			MessageBox(NULL, L"Not Find GameAssembly.dll", L"Init Error", MB_OK);
		}

#define DO_API(RetType,Name,Args)\
	Name = (Name##_t)GetProcAddress(hGameAssembly,#Name);
#include "il2cpp_api.h"
#undef DO_API

#define DO_FUNC(RetType,Name,Args,AssemblyName,Namespaze,ClassName,Func) \
	Name = (Name##_t)GetMethod(AssemblyName,Namespaze,ClassName,Func);
#include "il2cpp_function.h"
#undef DO_API

	}

	HWND GetHwnd() {
		return hWnd;
	}
	string GetTypeName(string Name) {
		size_t Idx = Name.find_last_of(".");
		if (Idx != string::npos)
		{
			Name = Name.substr(Idx + 1);
		}
		return Name;
	}
	string GetTypeName(const Il2CppType* pType) {
		return GetTypeName(il2cpp_type_get_name(pType));
	}
	void* GetMethod(string AssemblyName, string Namespaze, string ClassName, string Func) {
		Il2CppDomain* pDomain = il2cpp_domain_get();
		const Il2CppAssembly* pAssembly = il2cpp_domain_assembly_open(pDomain, AssemblyName.c_str());
		const Il2CppImage* pImage = il2cpp_assembly_get_image(pAssembly);
		Il2CppClass* pClass = il2cpp_class_from_name(pImage, Namespaze.c_str(), ClassName.c_str());
		void* Iter = NULL;
		while (true) {
			const MethodInfo* pMethod = il2cpp_class_get_methods(pClass, &Iter);
			if (pMethod == NULL) {
				break;
			}
			string temp = GetTypeName(il2cpp_method_get_return_type(pMethod));
			temp += " ";
			temp += il2cpp_method_get_name(pMethod);
			temp += "(";
			uint32_t MaxParam = il2cpp_method_get_param_count(pMethod);
			for (size_t i = 0; i < MaxParam; i++) {
				temp += GetTypeName(il2cpp_method_get_param(pMethod, i));
				temp += " ";
				temp += il2cpp_method_get_param_name(pMethod, i);
				if (MaxParam - 1 == i) {
					break;
				}
				temp += ",";
			}
			temp += ");";
			if (temp == Func) {
				//printf("%s\n", temp.c_str());
				return *(void**)pMethod;
			}
		}
		printf("error ClassName: %s Func: %s\n", ClassName.c_str(), Func.c_str());
		return NULL;
	}

	// 新增：通过方法名直接获取方法（适合无参数或简单方法）
	void* GetMethodByName(string AssemblyName, string Namespaze, string ClassName, string MethodName, int paramCount = 0) {
		Il2CppDomain* pDomain = il2cpp_domain_get();
		const Il2CppAssembly* pAssembly = il2cpp_domain_assembly_open(pDomain, AssemblyName.c_str());
		const Il2CppImage* pImage = il2cpp_assembly_get_image(pAssembly);
		Il2CppClass* pClass = il2cpp_class_from_name(pImage, Namespaze.c_str(), ClassName.c_str());
		const MethodInfo* pMethod = il2cpp_class_get_method_from_name(pClass, MethodName.c_str(), paramCount);
		if (pMethod != NULL) {
			return *(void**)pMethod;
		}
		printf("error finding method %s\n", MethodName.c_str());
		return NULL;
	}
	Il2CppString* create_il2cpp_string(const wchar_t* src) {
		return il2cpp_string_new_utf16(src, static_cast<int32_t>(wcslen(src)));
	}
}

