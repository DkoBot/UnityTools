#pragma once
#include "engine.h"
#include "base.h"
#include "../Misc/ColorInfo/InfoMesseng.h"
#include "memoryrendering.h"
#include <vector>
#include <string>

typedef struct Il2CppGlobalMetadataHeader
{
    int32_t sanity;
    int32_t version;
    int32_t stringLiteralOffset; // string data for managed code
    int32_t stringLiteralSize;
    int32_t stringLiteralDataOffset;
    int32_t stringLiteralDataSize;
    int32_t stringOffset; // string data for metadata
    int32_t stringSize;
    int32_t eventsOffset; // Il2CppEventDefinition
    int32_t eventsSize;
    int32_t propertiesOffset; // Il2CppPropertyDefinition
    int32_t propertiesSize;
    int32_t methodsOffset; // Il2CppMethodDefinition
    int32_t methodsSize;
    int32_t parameterDefaultValuesOffset; // Il2CppParameterDefaultValue
    int32_t parameterDefaultValuesSize;
    int32_t fieldDefaultValuesOffset; // Il2CppFieldDefaultValue
    int32_t fieldDefaultValuesSize;
    int32_t fieldAndParameterDefaultValueDataOffset; // uint8_t
    int32_t fieldAndParameterDefaultValueDataSize;
    int32_t fieldMarshaledSizesOffset; // Il2CppFieldMarshaledSize
    int32_t fieldMarshaledSizesSize;
    int32_t parametersOffset; // Il2CppParameterDefinition
    int32_t parametersSize;
    int32_t fieldsOffset; // Il2CppFieldDefinition
    int32_t fieldsSize;
    int32_t genericParametersOffset; // Il2CppGenericParameter
    int32_t genericParametersSize;
    int32_t genericParameterConstraintsOffset; // TypeIndex
    int32_t genericParameterConstraintsSize;
    int32_t genericContainersOffset; // Il2CppGenericContainer
    int32_t genericContainersSize;
    int32_t nestedTypesOffset; // TypeDefinitionIndex
    int32_t nestedTypesSize;
    int32_t interfacesOffset; // TypeIndex
    int32_t interfacesSize;
    int32_t vtableMethodsOffset; // EncodedMethodIndex
    int32_t vtableMethodsSize;
    int32_t interfaceOffsetsOffset; // Il2CppInterfaceOffsetPair
    int32_t interfaceOffsetsSize;
    int32_t typeDefinitionsOffset; // Il2CppTypeDefinition
    int32_t typeDefinitionsSize;
    int32_t imagesOffset; // Il2CppImageDefinition
    int32_t imagesSize;
    int32_t assembliesOffset; // Il2CppAssemblyDefinition
    int32_t assembliesSize;
    int32_t fieldRefsOffset; // Il2CppFieldRef
    int32_t fieldRefsSize;
    int32_t referencedAssembliesOffset; // int32_t
    int32_t referencedAssembliesSize;
    int32_t attributeDataOffset;
    int32_t attributeDataSize;
    int32_t attributeDataRangeOffset;
    int32_t attributeDataRangeSize;
    int32_t unresolvedIndirectCallParameterTypesOffset; // TypeIndex
    int32_t unresolvedIndirectCallParameterTypesSize;
    int32_t unresolvedIndirectCallParameterRangesOffset; // Il2CppMetadataRange
    int32_t unresolvedIndirectCallParameterRangesSize;
    int32_t windowsRuntimeTypeNamesOffset; // Il2CppWindowsRuntimeTypeNamePair
    int32_t windowsRuntimeTypeNamesSize;
    int32_t windowsRuntimeStringsOffset; // const char*
    int32_t windowsRuntimeStringsSize;
    int32_t exportedTypeDefinitionsOffset; // TypeDefinitionIndex
    int32_t exportedTypeDefinitionsSize;
} Il2CppGlobalMetadataHeader;

class globalmanagement
{
public:

	// 存储Assembly名称的数组
	static vector<string> assemblies;
	// 存储类名称的数组
	static vector<string> classes;
	// 存储类名称的小写版本，用于快速搜索
	static vector<string> classes_lower;

	// 变量信息结构
	struct VariableInfo {
		string name;
		string type;
		int offset;
	};

	// 方法信息结构
	struct ManagedMethodInfo {
		string name;
		string returnType;
		vector<string> parameters;
		bool isStatic;
	};

	// 存储变量和方法信息的数组
	static vector<VariableInfo> variables;
	static vector<ManagedMethodInfo> methods;

	// 缓存机制 - 避免重复获取数据
	static string cached_class_key; // 当前缓存的类名
	static vector<VariableInfo> cached_variables;
	static vector<ManagedMethodInfo> cached_methods;
	static vector<string> cached_method_params; // 缓存方法参数字符串

	// 排序缓存 - 避免每次渲染都排序
	static vector<string> sorted_assemblies; // 排序后的程序集列表
	static vector<string> sorted_classes; // 排序后的类列表
	static vector<ManagedMethodInfo> sorted_methods; // 排序后的方法列表
	static vector<string> sorted_method_params; // 对应的排序参数字符串
	static bool assemblies_need_sort; // 标记是否需要重新排序
	static bool classes_need_sort; // 标记是否需要重新排序
	static bool methods_need_sort; // 标记是否需要重新排序

	// 搜索缓存 - 提高搜索性能
	static vector<size_t> class_search_results; // 搜索匹配的类名索引（排序后的索引）
	static vector<size_t> original_to_sorted_indices; // 原始索引到排序索引的映射
	static string last_search_term; // 上次搜索词
	static int last_filter_type; // 上次筛选类型
	static bool search_results_valid; // 搜索结果是否有效

	static bool initialization();
	static bool get_Assemblies_Class(string Assemblies);
	static bool get_Class_Variable_Function(string Class);
	static vector<string> get_Class_object(string Class,string Assemblies);
	static string get_Class_Structure(string Class, string Assemblies);

	// 搜索优化函数
	static void update_class_search_results(const string& search_term, int filter_type = 0);

    static int32_t CalculateMetadataSize(Il2CppGlobalMetadataHeader* header);
    static bool DumpMetadataToFile(uintptr_t metadataAddr, int32_t size, const char* filename);
};

