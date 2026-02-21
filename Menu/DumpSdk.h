#pragma once
#include "base.h"
#include "engine.h"
#include "../Misc/ColorInfo/InfoMesseng.h"
#include <vector>
#include <fstream>
#include <sstream>
class DumpSdk
{
protected:
	inline static HMODULE hModule = nullptr;
public:
	static size_t il2cpp_EnummAssembly(std::vector<Il2CppAssembly*>& Assemblys)
	{
		size_t nrofassemblies = 0;
		Il2CppAssembly** assemblies = (Il2CppAssembly**)il2cpp_domain_get_assemblies(il2cpp_domain_get(), &nrofassemblies);
		for (int i = 0; i < nrofassemblies; i++)
			Assemblys.push_back(assemblies[i]);
		return Assemblys.size();
	}
	static Il2CppImage* il2cpp_GetImage(Il2CppAssembly* Assembly)
	{
		return (Il2CppImage*)il2cpp_assembly_get_image(Assembly);
	}
	static string il2cpp_GetImageName(Il2CppImage* Image)
	{
		return il2cpp_image_get_name(Image);
	}
	static size_t il2cpp_GetClassCount(Il2CppImage* Image)
	{
		return il2cpp_image_get_class_count(Image);
	}
	static size_t il2cpp_EnumClasses(Il2CppImage* Image, std::vector<Il2CppClass*>& classes)
	{
		size_t count = il2cpp_image_get_class_count(Image);
		classes.reserve(count);
		for (size_t i = 0; i < count; i++)
		{
			Il2CppClass* Class = (Il2CppClass*)il2cpp_image_get_class(Image, i);
			if (!Class) continue;
			classes.push_back(Class);
		}
		return classes.size();
	}
	static string il2cpp_GetClassNamespace(Il2CppClass* klass)
	{
		return il2cpp_class_get_namespace(klass);
	}
	static string il2cpp_GetClassName(Il2CppClass* klass)
	{
		return il2cpp_class_get_name(klass);
	}
	static size_t il2cpp_EnumFields(Il2CppClass* klass, std::vector<FieldInfo*>& Fields)
	{
		void* iter = NULL;
		FieldInfo* field;
		do
		{
			field = il2cpp_class_get_fields(klass, &iter);
			if (!field) continue;
			Fields.push_back(field);
		} while (field);
		return Fields.size();
	}
	static string il2cpp_GetTypeName(Il2CppType* type)
	{
		return il2cpp_type_get_name(type);
	}
	static Il2CppType* il2cpp_GetFieldType(FieldInfo* field)
	{
		return (Il2CppType*)il2cpp_field_get_type(field);
	}
	static string il2cpp_GetFieldName(FieldInfo* field)
	{
		return il2cpp_field_get_name(field);
	}
	static size_t il2cpp_GetFieldOffset(FieldInfo* field)
	{
		return il2cpp_field_get_offset(field);
	}
	static size_t il2cpp_EnumMethods(Il2CppClass* klass, std::vector<MethodInfo*>& Methods)
	{
		void* iter = NULL;
		MethodInfo* method;
		do
		{
			method = (MethodInfo*)il2cpp_class_get_methods(klass, &iter);
			if (!method) continue;
			Methods.push_back(method);
		} while (method);
		return Methods.size();
	}
	static DWORD_PTR il2cpp_GetMethodAddress(MethodInfo* method)
	{

		if (!method) return 0;
		return *reinterpret_cast<DWORD_PTR*>(method);
	}
	static Il2CppType* il2cpp_GetMethodRetType(MethodInfo* Method)
	{
		return (Il2CppType*)il2cpp_method_get_return_type(Method);
	}
	static string il2cpp_GetMethodName(MethodInfo* Method)
	{
		return il2cpp_method_get_name(Method);
	}
	static size_t il2cpp_GetMethodParamCount(MethodInfo* Method)
	{
		return il2cpp_method_get_param_count(Method);
	}
	static Il2CppType* il2cpp_GetMethodParam(MethodInfo* Method, size_t index)
	{
		return (Il2CppType*)il2cpp_method_get_param(Method, (uint32_t)index);
	}
	static string il2cpp_GetMethodParamName(MethodInfo* Method, size_t index)
	{
		return il2cpp_method_get_param_name(Method, (uint32_t)index);
	}

	static void il2cpp_dump_init(HMODULE Module);
	static bool il2cpp_Dump2File(string file);
};

