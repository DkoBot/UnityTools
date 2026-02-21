#include "globalmanagement.h"
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <tuple>

// 初始化assemblies变量
vector<string> globalmanagement::assemblies;

// 初始化classes变量
vector<string> globalmanagement::classes;
vector<string> globalmanagement::classes_lower;

// 初始化variables和methods变量
vector<globalmanagement::VariableInfo> globalmanagement::variables;
vector<globalmanagement::ManagedMethodInfo> globalmanagement::methods;

// 初始化缓存变量
string globalmanagement::cached_class_key = "";
vector<globalmanagement::VariableInfo> globalmanagement::cached_variables;
vector<globalmanagement::ManagedMethodInfo> globalmanagement::cached_methods;
vector<string> globalmanagement::cached_method_params;

// 初始化排序缓存变量
vector<string> globalmanagement::sorted_assemblies;
vector<string> globalmanagement::sorted_classes;
vector<globalmanagement::ManagedMethodInfo> globalmanagement::sorted_methods;
vector<string> globalmanagement::sorted_method_params;
bool globalmanagement::assemblies_need_sort = true;
bool globalmanagement::classes_need_sort = true;
bool globalmanagement::methods_need_sort = true;

// 初始化搜索缓存变量
vector<size_t> globalmanagement::class_search_results;
vector<size_t> globalmanagement::original_to_sorted_indices;
string globalmanagement::last_search_term = "";
int globalmanagement::last_filter_type = 0;
bool globalmanagement::search_results_valid = false;


bool globalmanagement::initialization() {
	// 清空之前的列表
	assemblies.clear();

	// 获取IL2CPP域
	Il2CppDomain* domain = il2cpp_domain_get();
	if (!domain) {
		return false;
	}

	// 获取当前域中的所有程序集
	size_t assemblyCount = 0;
	const Il2CppAssembly** allAssemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);

	if (!allAssemblies || assemblyCount == 0) {
		return false;
	}

	// 遍历所有程序集并获取名称
	for (size_t i = 0; i < assemblyCount; i++) {
		const Il2CppAssembly* assembly = allAssemblies[i];
		if (assembly) {
			// 获取程序集的镜像
			const Il2CppImage* image = il2cpp_assembly_get_image(assembly);
			if (image) {
				// 使用API函数获取镜像名称
				const char* imageName = il2cpp_image_get_name(image);
				if (imageName) {
					// 将程序集名称添加到列表中
					assemblies.push_back(string(imageName));
				}
			}
		}
	}

	// 如果找到了至少一个程序集，则初始化成功
	if (!assemblies.empty()) {
		// 预排序程序集列表
		sorted_assemblies = assemblies;
		std::sort(sorted_assemblies.begin(), sorted_assemblies.end());
		assemblies_need_sort = false;
	}
	return !assemblies.empty();
}
bool globalmanagement::get_Assemblies_Class(string Assemblies) {
	// 清空之前的类列表
	classes.clear();
	classes_lower.clear();

	// 获取IL2CPP域
	Il2CppDomain* domain = il2cpp_domain_get();
	if (!domain) {
		return false;
	}

	// 打开指定的程序集
	const Il2CppAssembly* assembly = il2cpp_domain_assembly_open(domain, Assemblies.c_str());
	if (!assembly) {
		return false;
	}

	// 获取程序集的镜像
	const Il2CppImage* image = il2cpp_assembly_get_image(assembly);
	if (!image) {
		return false;
	}

	// 获取镜像中的类数量
	size_t classCount = il2cpp_image_get_class_count(image);
	if (classCount == 0) {
		return false;
	}

	// 遍历所有类并获取类名和命名空间
	for (size_t i = 0; i < classCount; i++) {
		const Il2CppClass* klass = il2cpp_image_get_class(image, i);
		if (klass) {
			// 获取类名
			const char* className = il2cpp_class_get_name(klass);
			// 获取命名空间
			const char* classNamespace = il2cpp_class_get_namespace(klass);

			if (className) {
				string fullClassName;

				// 如果存在命名空间，则组合为 Namespace.Class 格式
				if (classNamespace && strlen(classNamespace) > 0) {
					fullClassName = string(classNamespace) + "." + string(className);
				} else {
					// 如果没有命名空间，直接使用类名
					fullClassName = string(className);
				}

				// 对于特殊的类名（如 <>c），尝试获取更多上下文信息
				string enhancedClassName = fullClassName;
				if (strcmp(className, "<>c") == 0) {
					// 尝试获取父类信息
					const Il2CppClass* parentClass = il2cpp_class_get_parent(klass);
					if (parentClass) {
						const char* parentName = il2cpp_class_get_name(parentClass);
						const char* parentNamespace = il2cpp_class_get_namespace(parentClass);
						if (parentName) {
							string parentFullName;
							if (parentNamespace && strlen(parentNamespace) > 0) {
								parentFullName = string(parentNamespace) + "." + string(parentName);
							} else {
								parentFullName = string(parentName);
							}
							enhancedClassName = parentFullName + "." + fullClassName;
						}
					}

					// 如果没有父类信息，尝试获取声明类型
					if (enhancedClassName == fullClassName) {
						const Il2CppClass* declaringType = il2cpp_class_get_declaring_type(klass);
						if (declaringType) {
							const char* declaringName = il2cpp_class_get_name(declaringType);
							const char* declaringNamespace = il2cpp_class_get_namespace(declaringType);
							if (declaringName) {
								string declaringFullName;
								if (declaringNamespace && strlen(declaringNamespace) > 0) {
									declaringFullName = string(declaringNamespace) + "." + string(declaringName);
								} else {
									declaringFullName = string(declaringName);
								}
								enhancedClassName = declaringFullName + "." + fullClassName;
							}
						}
					}
				}

				// 将增强后的类名添加到列表中
				classes.push_back(enhancedClassName);

				// 同时添加小写版本用于快速搜索
				string lowerClassName = fullClassName;
				// 使用循环代替std::transform，避免编译器兼容性问题
				for (size_t i = 0; i < lowerClassName.size(); ++i) {
					lowerClassName[i] = ::tolower(lowerClassName[i]);
				}
				classes_lower.push_back(lowerClassName);
			}
		}
	}

	// 如果找到了至少一个类，则获取成功
	if (!classes.empty()) {
		// 创建原始索引到排序索引的映射
		original_to_sorted_indices.resize(classes.size());
		for (size_t i = 0; i < classes.size(); ++i) {
			original_to_sorted_indices[i] = i;
		}

		// 预排序类列表（同时排序索引映射）
		sorted_classes = classes;
		std::sort(sorted_classes.begin(), sorted_classes.end());

		// 对索引映射进行相应的排序
		std::sort(original_to_sorted_indices.begin(), original_to_sorted_indices.end(),
			[&](size_t a, size_t b) {
				return classes[a] < classes[b];
			});

		classes_need_sort = false;

		// 更新classes_lower向量（保持原始顺序，与原始classes对应）
		classes_lower.resize(classes.size());
		for (size_t i = 0; i < classes.size(); ++i) {
			classes_lower[i] = classes[i];
			// 使用循环代替std::transform，避免编译器兼容性问题
			for (size_t j = 0; j < classes_lower[i].size(); ++j) {
				classes_lower[i][j] = ::tolower(classes_lower[i][j]);
			}
		}

		// 重置搜索结果
		search_results_valid = false;
	}
	return !classes.empty();
}
bool globalmanagement::get_Class_Variable_Function(string Class) {
	// 检查缓存是否可用
	if (cached_class_key == Class && !cached_variables.empty()) {
		// 使用缓存的数据
		variables = cached_variables;
		methods = cached_methods;
		// 注意：cached_method_params 已经在缓存更新时构建
		return true;
	}

	// 清空之前的变量和方法列表
	variables.clear();
	methods.clear();

	// 解析类名和命名空间
	string className = Class;
	string namespaceName = "";

	// 检查是否有命名空间（通过最后一个点号分割）
	size_t lastDot = Class.find_last_of('.');
	if (lastDot != string::npos) {
		namespaceName = Class.substr(0, lastDot);
		className = Class.substr(lastDot + 1);
	}

	// 获取IL2CPP域
	Il2CppDomain* domain = il2cpp_domain_get();
	if (!domain) {
		return false;
	}

	// 遍历所有程序集，查找包含该类的程序集
	const Il2CppAssembly** allAssemblies = nullptr;
	size_t assemblyCount = 0;
	allAssemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);

	if (!allAssemblies || assemblyCount == 0) {
		return false;
	}

	Il2CppClass* targetClass = nullptr;
	string foundAssemblyName = "";

	// 在所有程序集中查找目标类
	for (size_t i = 0; i < assemblyCount && !targetClass; i++) {
		const Il2CppAssembly* assembly = allAssemblies[i];
		if (assembly) {
			const Il2CppImage* image = il2cpp_assembly_get_image(assembly);
			if (image) {
				// 尝试获取类
				targetClass = il2cpp_class_from_name(image, namespaceName.c_str(), className.c_str());
				if (targetClass) {
					// 找到类时，存储对应的程序集名称
					const char* imageName = il2cpp_image_get_name(image);
					if (imageName) {
						foundAssemblyName = string(imageName) + ".dll";
					}
				}
			}
		}
	}

	if (!targetClass) {
		return false;
	}

	// 获取变量信息
	void* fieldIter = nullptr;
	FieldInfo* field = nullptr;
	while ((field = il2cpp_class_get_fields(targetClass, &fieldIter)) != nullptr) {
		if (field && field->name) {
			VariableInfo varInfo;

			// 获取变量名称
			varInfo.name = string(field->name);

			// 获取变量类型
			if (field->type) {
				const char* typeName = il2cpp_type_get_name(field->type);
				varInfo.type = typeName ? string(typeName) : "Unknown";
			} else {
				varInfo.type = "Unknown";
			}

			// 获取变量偏移
			varInfo.offset = il2cpp_field_get_offset(field);
			// 不将偏移为 0x0 的变量加入表中
			if (varInfo.offset == 0)
				continue;

			variables.push_back(varInfo);
		}
	}

	// 获取方法信息
	void* methodIter = nullptr;
	const MethodInfo* method = nullptr;
	static int methodIndex = 0; // 用于生成唯一的方法标识符
	while ((method = il2cpp_class_get_methods(targetClass, &methodIter)) != nullptr) {
		ManagedMethodInfo methodInfo;

		// 优先使用API函数获取方法名称
		const char* apiMethodName = il2cpp_method_get_name(method);
		if (apiMethodName && strlen(apiMethodName) > 0 && strlen(apiMethodName) < 256) {
			// 检查是否包含不可打印字符
			bool hasInvalidChars = false;
			for (size_t i = 0; i < strlen(apiMethodName); i++) {
				if (apiMethodName[i] < 32 && apiMethodName[i] != '\0') {
					hasInvalidChars = true;
					break;
				}
			}

			if (!hasInvalidChars) {
				methodInfo.name = string(apiMethodName);
			}
			else {
				methodInfo.name = "Invalid_Method_Name_" + to_string(methodIndex++);
			}
		}
		else {
			// 如果API函数失败，尝试直接访问结构体成员
			if (method->name && strlen(method->name) > 0 && strlen(method->name) < 256) {
				methodInfo.name = string(method->name);
			}
			else {
				methodInfo.name = "Unknown_Method_" + to_string(methodIndex++);
			}
		}

		// 获取返回值类型
		const Il2CppType* returnType = il2cpp_method_get_return_type(method);
		if (returnType) {
			const char* returnTypeName = il2cpp_type_get_name(returnType);
			if (returnTypeName && strlen(returnTypeName) > 0 && strlen(returnTypeName) < 256) {
				methodInfo.returnType = string(returnTypeName);
			}
			else {
				methodInfo.returnType = "Unknown";
			}
		}
		else {
			methodInfo.returnType = "Void";
		}

		// 获取参数信息
		int paramCount = il2cpp_method_get_param_count(method);
		for (int i = 0; i < paramCount; i++) {
			const Il2CppType* paramType = il2cpp_method_get_param(method, i);
			if (paramType) {
				const char* paramTypeName = il2cpp_type_get_name(paramType);
				const char* paramName = il2cpp_method_get_param_name(method, i);

				string typeStr = "Unknown";
				if (paramTypeName && strlen(paramTypeName) > 0 && strlen(paramTypeName) < 256) {
					typeStr = string(paramTypeName);
				}

				string nameStr = "param" + to_string(i);
				if (paramName && strlen(paramName) > 0 && strlen(paramName) < 256) {
					nameStr = string(paramName);
				}

				string paramInfo = typeStr + " " + nameStr;
				methodInfo.parameters.push_back(paramInfo);
			}
		}

		// 检查是否为静态方法
		uint32_t methodFlags = il2cpp_method_get_flags(method, nullptr); // 有些IL2CPP版本使用这个签名
		methodInfo.isStatic = (methodFlags & 0x0010) != 0;

		methods.push_back(methodInfo);
	}

	// 更新缓存
	if (!variables.empty() || !methods.empty()) {
		cached_class_key = Class;
		cached_variables = variables;
		cached_methods = methods;

		// 构建方法参数字符串缓存
		cached_method_params.resize(methods.size());
		for (size_t i = 0; i < methods.size(); ++i) {
			const auto& method = methods[i];
			string& paramsStr = cached_method_params[i];
			paramsStr.clear();
			for (size_t j = 0; j < method.parameters.size(); j++) {
				if (j > 0) paramsStr += ", ";
				paramsStr += method.parameters[j];
			}
		}

		// 预排序方法列表
		sorted_methods = methods;
		std::sort(sorted_methods.begin(), sorted_methods.end(),
			[](const globalmanagement::ManagedMethodInfo& a, const globalmanagement::ManagedMethodInfo& b) {
				return a.name < b.name;
			});

		// 构建排序后的参数字符串列表
		sorted_method_params.resize(sorted_methods.size());
		for (size_t i = 0; i < sorted_methods.size(); ++i) {
			const auto& method = sorted_methods[i];
			// 找到原始方法在缓存中的参数字符串
			auto it = std::find_if(methods.begin(), methods.end(),
				[&method](const globalmanagement::ManagedMethodInfo& m) {
					return m.name == method.name && m.returnType == method.returnType &&
						   m.parameters == method.parameters && m.isStatic == method.isStatic;
				});
			if (it != methods.end()) {
				size_t originalIndex = std::distance(methods.begin(), it);
				if (originalIndex < cached_method_params.size()) {
					sorted_method_params[i] = cached_method_params[originalIndex];
				} else {
					sorted_method_params[i] = "";
				}
			} else {
				sorted_method_params[i] = "";
			}
		}

		methods_need_sort = false;
	}

	// 如果至少获取到了变量或方法，则返回成功
	return true;
}
// 更新类搜索结果缓存
void globalmanagement::update_class_search_results(const string& search_term, int filter_type) {
	// 如果搜索词和筛选类型都没有改变且结果仍然有效，直接返回
	if (last_search_term == search_term && search_results_valid && last_filter_type == filter_type) {
		return;
	}

	// 更新搜索词和筛选类型
	last_search_term = search_term;
	last_filter_type = filter_type;
	class_search_results.clear();

	// 将搜索词转换为小写（如果不为空）
	string search_lower;
	if (!search_term.empty()) {
		search_lower = search_term;
		for (size_t i = 0; i < search_lower.size(); ++i) {
			search_lower[i] = ::tolower(search_lower[i]);
		}
	}

	// 在原始classes中搜索，然后通过映射转换为排序后的索引
	for (size_t original_idx = 0; original_idx < classes.size(); ++original_idx) {
		const string& class_name = classes[original_idx];

		// 首先检查筛选类型
		bool matches_filter = false;
		switch (filter_type) {
			case 0: // All
				matches_filter = true;
				break;
			case 1: // System
				matches_filter = (class_name.find("System.") == 0);
				break;
			case 2: // Unity
				matches_filter = (class_name.find("UnityEngine.") == 0 ||
								class_name.find("Unity.") == 0 ||
								class_name.find("TMPro.") == 0);
				break;
			case 3: // Custom
				matches_filter = (class_name.find("System.") != 0 &&
								class_name.find("UnityEngine.") != 0 &&
								class_name.find("Unity.") != 0 &&
								class_name.find("TMPro.") != 0);
				break;
		}

		if (!matches_filter) {
			continue;
		}

		// 然后检查搜索词
		bool matches_search = search_term.empty() || (classes_lower[original_idx].find(search_lower) != string::npos);

		if (matches_search) {
			// 找到原始索引对应的排序后索引
			auto it = std::find(original_to_sorted_indices.begin(), original_to_sorted_indices.end(), original_idx);
			if (it != original_to_sorted_indices.end()) {
				size_t sorted_idx = std::distance(original_to_sorted_indices.begin(), it);
				class_search_results.push_back(sorted_idx);
			}
		}
	}

	search_results_valid = true;
}

vector<string> globalmanagement::get_Class_object(string Class, string Assemblies) {
	// 解析类名和命名空间
	size_t dotCount = 0;
	size_t lastDotPos = string::npos;
	string temp_selNamespace = "";
	string temp_selClass = "";
	// 统计点号数量并记录最后一个点号位置
	for (size_t i = 0; i < Class.length(); i++) {
		if (Class[i] == '.') {
			dotCount++;
			lastDotPos = i;
		}
	}
	if (dotCount == 1) {
		// 只有一个点号，说明这是全局类（格式：ClassName）
		temp_selNamespace = "";
		// g_selClass 已经是完整的类名
	}
	else if (dotCount > 1 && lastDotPos != string::npos) {
		// 有多个点号，通过最后一个点号分割
		temp_selNamespace = Class.substr(0, lastDotPos);  // 命名空间部分
		temp_selClass = Class.substr(lastDotPos + 1);    // 类名部分
	}
	else {
		// 没有点号或异常情况
		temp_selNamespace = "";
		// g_selClass 保持不变
	}
	vector<string> objects = {};
	try {
		il2cpp_thread_attach(il2cpp_domain_get());
		// 构造完整的类型字符串：命名空间.类名, 程序集名
		string fullTypeName = Class + ", " + Assemblies;
		Il2CppString* typeName = Engine::create_il2cpp_string(std::wstring(fullTypeName.begin(), fullTypeName.end()).c_str());
		void* temp_Type = GetType(typeName);
		if (temp_Type) {
			// 使用FindObjectsOfTypeAll来查找所有对象，包括未激活的
			Il2CppArray* arrlist = (Il2CppArray*)FindObjectsOfTypeAll(temp_Type);
			if (arrlist && arrlist != (Il2CppArray*)0xCCCCCCCCCCCCCCCC && arrlist->max_length > 0 && arrlist->vector) {
				for (int i = 0; i < arrlist->max_length; ++i) {
					void* component = ((void**)arrlist->vector)[i];
					void* gameObject = GetGameObject(component);
					GameObject* go = (GameObject*)gameObject;
					void* gameObjects = go->GetComponent(temp_selClass);
					if (gameObjects) {
					// 将指针地址转换为十六进制字符串（不带0x前缀）
					char addressStr[32];
					sprintf_s(addressStr, sizeof(addressStr), "%llx", (uintptr_t)gameObjects);
					// 如果不是空指针（0000000000000000），才添加到列表中
					if (strcmp(addressStr, "0000000000000000") != 0) {
						objects.push_back(string(addressStr));
					}
					}
				}
			}
		}
		return objects;
	}
	catch (...) {
		return objects;
	}
}

string globalmanagement::get_Class_Structure(string Class, string Assemblies) {
	// 解析类名和命名空间
	string className = Class;
	string namespaceName = "";

	// 检查是否有命名空间（通过最后一个点号分割）
	size_t lastDot = Class.find_last_of('.');
	if (lastDot != string::npos) {
		namespaceName = Class.substr(0, lastDot);
		className = Class.substr(lastDot + 1);
		
		// 如果命名空间以 "None." 开头，移除 "None." 前缀
		if (namespaceName.length() >= 5 && namespaceName.substr(0, 5) == "None.") {
			namespaceName = namespaceName.substr(5);
		}
		// 如果命名空间就是 "None"，清空它
		if (namespaceName == "None") {
			namespaceName = "";
		}
	}

	// 获取IL2CPP域
	Il2CppDomain* domain = il2cpp_domain_get();
	if (!domain) {
		return "// Error: Cannot get IL2CPP domain";
	}

	// 打开指定的程序集
	const Il2CppAssembly* assembly = il2cpp_domain_assembly_open(domain, Assemblies.c_str());
	if (!assembly) {
		return "// Error: Cannot open assembly: " + Assemblies;
	}

	// 获取程序集的镜像
	const Il2CppImage* image = il2cpp_assembly_get_image(assembly);
	if (!image) {
		return "// Error: Cannot get image from assembly: " + Assemblies;
	}

	// 获取目标类
	Il2CppClass* targetClass = il2cpp_class_from_name(image, namespaceName.c_str(), className.c_str());
	if (!targetClass) {
		return "// Error: Cannot find class: " + Class + " in assembly: " + Assemblies;
	}

	// 构建类结构字符串
	string result = "class ";
	result += className;
	result += " {\npublic:\n";

	// 获取变量信息并排序
	void* fieldIter = nullptr;
	FieldInfo* field = nullptr;
	vector<tuple<int32_t, string, string>> fieldList; // offset, type, name

	while ((field = il2cpp_class_get_fields(targetClass, &fieldIter)) != nullptr) {
		if (field && field->name) {
			// 获取变量偏移
			int32_t offset = il2cpp_field_get_offset(field);

			// 忽略偏移为0x00的字段
			if (offset == 0) {
				continue;
			}

			// 获取变量类型
			string typeStr = "Unknown";
			if (field->type) {
				const char* typeName = il2cpp_type_get_name(field->type);
				if (typeName) {
					typeStr = string(typeName);
				}
			}

			// 获取变量名称
			string fieldName = string(field->name);

			// 添加到列表中
			fieldList.emplace_back(offset, typeStr, fieldName);
		}
	}

	// 按照偏移量从低到高排序
	sort(fieldList.begin(), fieldList.end(), [](const auto& a, const auto& b) {
		return get<0>(a) < get<0>(b);
	});

	// 生成格式化的输出
	bool hasFields = false;
	for (const auto& fieldTuple : fieldList) {
		hasFields = true;
		int32_t offset = get<0>(fieldTuple);
		const string& typeStr = get<1>(fieldTuple);
		const string& fieldName = get<2>(fieldTuple);

		// 格式化为C++风格
		result += "\t";
		result += typeStr;
		result += " ";
		result += fieldName;
		result += "; // 0x";
		char offsetStr[16];
		sprintf_s(offsetStr, sizeof(offsetStr), "%X", offset);
		result += offsetStr;
		result += "\n";
	}

	if (!hasFields) {
		result += "\t// No fields found\n";
	}

	// 获取方法信息
	void* methodIter = nullptr;
	const MethodInfo* method = nullptr;
	vector<string> methodList;

	while ((method = il2cpp_class_get_methods(targetClass, &methodIter)) != nullptr) {
		if (method) {
			// 优先使用API函数获取方法名称
			const char* apiMethodName = il2cpp_method_get_name(method);
			string methodName = "";
			
			if (apiMethodName && strlen(apiMethodName) > 0 && strlen(apiMethodName) < 256) {
				// 检查是否包含不可打印字符
				bool hasInvalidChars = false;
				for (size_t i = 0; i < strlen(apiMethodName); i++) {
					if (apiMethodName[i] < 32 && apiMethodName[i] != '\0') {
						hasInvalidChars = true;
						break;
					}
				}
				
				if (!hasInvalidChars) {
					methodName = string(apiMethodName);
				} else {
					// 跳过无效的方法名（通常是内部方法）
					continue;
				}
			} else {
				// 如果API函数失败，尝试直接访问结构体成员
				if (method->name && strlen(method->name) > 0 && strlen(method->name) < 256) {
					methodName = string(method->name);
				} else {
					// 跳过无效的方法
					continue;
				}
			}
			
			// 跳过编译器生成的方法（通常以 < 或 .ctor/.cctor 开头）
			if (methodName.empty() || methodName[0] == '<' || 
				methodName == ".ctor" || methodName == ".cctor") {
				continue;
			}

			// 获取返回类型
			string returnType = "Void";
			const Il2CppType* returnTypePtr = il2cpp_method_get_return_type(method);
			if (returnTypePtr) {
				char* typeName = il2cpp_type_get_name(returnTypePtr);
				if (typeName) {
					returnType = string(typeName);
				}
			}

			// 获取参数
			uint32_t paramCount = il2cpp_method_get_param_count(method);
			string paramList = "";
			for (uint32_t i = 0; i < paramCount; i++) {
				const Il2CppType* paramType = il2cpp_method_get_param(method, i);
				const char* paramName = il2cpp_method_get_param_name(method, i);

				if (paramType) {
					char* typeName = il2cpp_type_get_name(paramType);
					if (typeName) {
						if (i > 0) {
							paramList += ", ";
						}
						paramList += string(typeName);
						
						// 检查参数名是否有效
						if (paramName && strlen(paramName) > 0 && strlen(paramName) < 256) {
							// 检查是否包含不可打印字符
							bool hasInvalidChars = false;
							for (size_t j = 0; j < strlen(paramName); j++) {
								if (paramName[j] < 32 && paramName[j] != '\0') {
									hasInvalidChars = true;
									break;
								}
							}
							
							if (!hasInvalidChars) {
								paramList += " ";
								paramList += string(paramName);
							} else {
								// 如果参数名无效，使用默认名称
								paramList += " param" + to_string(i);
							}
						} else {
							// 如果没有参数名，使用默认名称
							paramList += " param" + to_string(i);
						}
					}
				}
			}

			// 构建方法签名
			string methodSig = "\t";
			methodSig += returnType;
			methodSig += " ";
			methodSig += methodName;
			methodSig += "(";
			methodSig += paramList;
			methodSig += ");";

			methodList.push_back(methodSig);
		}
	}

	// 添加方法到结果
	if (!methodList.empty()) {
		result += "\n\t// Methods\n";
		for (const auto& methodSig : methodList) {
			result += methodSig;
			result += "\n";
		}
	}

	result += "};\n";

	return result;
}


int32_t globalmanagement::CalculateMetadataSize(Il2CppGlobalMetadataHeader* header) {
	int32_t maxOffset = 0;

	// 辅助 lambda
	auto check = [&](int32_t offset, int32_t size) {
		if (offset > 0 && offset + size > maxOffset)
			maxOffset = offset + size;
		};

	// 检查所有表
	check(header->stringLiteralOffset, header->stringLiteralSize);
	check(header->stringLiteralDataOffset, header->stringLiteralDataSize);
	check(header->stringOffset, header->stringSize);
	check(header->eventsOffset, header->eventsSize);
	check(header->propertiesOffset, header->propertiesSize);
	check(header->methodsOffset, header->methodsSize);
	check(header->parameterDefaultValuesOffset, header->parameterDefaultValuesSize);
	check(header->fieldDefaultValuesOffset, header->fieldDefaultValuesSize);
	check(header->fieldAndParameterDefaultValueDataOffset, header->fieldAndParameterDefaultValueDataSize);
	check(header->fieldMarshaledSizesOffset, header->fieldMarshaledSizesSize);
	check(header->parametersOffset, header->parametersSize);
	check(header->fieldsOffset, header->fieldsSize);
	check(header->genericParametersOffset, header->genericParametersSize);
	check(header->genericParameterConstraintsOffset, header->genericParameterConstraintsSize);
	check(header->genericContainersOffset, header->genericContainersSize);
	check(header->nestedTypesOffset, header->nestedTypesSize);
	check(header->interfacesOffset, header->interfacesSize);
	check(header->vtableMethodsOffset, header->vtableMethodsSize);
	check(header->interfaceOffsetsOffset, header->interfaceOffsetsSize);
	check(header->typeDefinitionsOffset, header->typeDefinitionsSize);
	check(header->imagesOffset, header->imagesSize);
	check(header->assembliesOffset, header->assembliesSize);
	check(header->fieldRefsOffset, header->fieldRefsSize);
	check(header->referencedAssembliesOffset, header->referencedAssembliesSize);
	check(header->attributeDataOffset, header->attributeDataSize);
	check(header->attributeDataRangeOffset, header->attributeDataRangeSize);
	check(header->unresolvedIndirectCallParameterTypesOffset, header->unresolvedIndirectCallParameterTypesSize);
	check(header->unresolvedIndirectCallParameterRangesOffset, header->unresolvedIndirectCallParameterRangesSize);
	check(header->windowsRuntimeTypeNamesOffset, header->windowsRuntimeTypeNamesSize);
	check(header->windowsRuntimeStringsOffset, header->windowsRuntimeStringsSize);
	check(header->exportedTypeDefinitionsOffset, header->exportedTypeDefinitionsSize);

	return maxOffset;
}

bool globalmanagement::DumpMetadataToFile(uintptr_t metadataAddr, int32_t size, const char* filename) {
	// 1. 打开目标进程（如果是外部进程）
	// HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, pid);
	// 如果是本进程，直接用 GetCurrentProcess()
	HANDLE hProcess = GetCurrentProcess();

	// 2. 分配缓冲区
	BYTE* buffer = (BYTE*)malloc(size);
	if (!buffer) {
		printf("[-] Failed to allocate buffer\n");
		return false;
	}

	// 3. 读取内存
	SIZE_T bytesRead;
	BOOL success = ReadProcessMemory(hProcess, (LPCVOID)metadataAddr, buffer, size, &bytesRead);

	if (!success || bytesRead != size) {
		printf("[-] ReadProcessMemory failed: %d\n", GetLastError());
		free(buffer);
		return false;
	}

	// 4. 写入文件
	FILE* file = fopen(filename, "wb");
	if (!file) {
		free(buffer);
		return false;
	}

	fwrite(buffer, 1, size, file);
	fclose(file);

	// 5. 验证 Magic
	uint32_t magic = *(uint32_t*)buffer;
	if (magic == 0xFAB11BAF) {
		return true;
	}
	else {
		InfoMesseng::ColorPrint("WARING", "Invalid magic dont is [FA B1 1B AF]",3);
	}

	free(buffer);
	return true;
}