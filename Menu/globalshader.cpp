#include "globalshader.h"
#include <cstring>
#include <cstdio>

vector<string> globalshader::shaderNames;
vector<string> globalshader::cachedShaderNames;
Shader* globalshader::temp_sle_Shader;

bool globalshader::init() {
    // 在清空之前，保存当前列表作为缓存
    cachedShaderNames = shaderNames;
    shaderNames.clear();
    
    try {
        il2cpp_thread_attach(il2cpp_domain_get());
        
        // 获取 Shader 类型
        string fullTypeName = "UnityEngine.Shader, UnityEngine.CoreModule";
        Il2CppString* typeName = Engine::create_il2cpp_string(wstring(fullTypeName.begin(), fullTypeName.end()).c_str());
        void* shaderType = GetType(typeName);
        
        if (!shaderType) {
            return false;
        }
        
        // 使用 FindObjectsOfTypeAll 获取所有 Shader 对象
        Il2CppArray* shaderArray = (Il2CppArray*)FindObjectsOfTypeAll(shaderType);
        
        if (!shaderArray || shaderArray == (Il2CppArray*)0xCCCCCCCCCCCCCCCC || shaderArray->max_length == 0 || !shaderArray->vector) {
            return false;
        }
        
        // 获取 get_name 方法
        Il2CppDomain* pDomain = il2cpp_domain_get();
        const Il2CppAssembly* pAssembly = il2cpp_domain_assembly_open(pDomain, "UnityEngine.CoreModule");
        if (!pAssembly) return false;
        
        const Il2CppImage* pImage = il2cpp_assembly_get_image(pAssembly);
        Il2CppClass* objectClass = il2cpp_class_from_name(pImage, "UnityEngine", "Object");
        if (!objectClass) return false;
        
        const MethodInfo* getNameMethod = il2cpp_class_get_method_from_name(objectClass, "get_name", 0);
        if (!getNameMethod) return false;
        
        // 遍历所有 Shader 对象并获取名称
        for (int i = 0; i < shaderArray->max_length; ++i) {
            void* shaderObj = ((void**)shaderArray->vector)[i];
            if (!shaderObj) continue;
            
            // 调用 get_name() 方法获取名称
            Il2CppObject* exc = nullptr;
            Il2CppString* nameStr = (Il2CppString*)il2cpp_runtime_invoke(getNameMethod, shaderObj, nullptr, &exc);
            
            if (nameStr && !exc) {
                string shaderName = Engine::il2cppStringToStdString(nameStr);
                if (!shaderName.empty()) {
                    shaderNames.push_back(shaderName);
                }
            }
        }
		return true;
    }
    catch (...) {
        // 异常处理
		return false;
    }
}
bool globalshader::get_shader_by_name(const string& name) {
    globalshader::temp_sle_Shader = NULL;
    try {
        il2cpp_thread_attach(il2cpp_domain_get());
        // 使用 FindShader 通过名称查找 Shader
        Il2CppString* shaderNameStr = Engine::create_il2cpp_string(wstring(name.begin(), name.end()).c_str());
        Shader* shaderObj = FindShader(shaderNameStr);
        
        if (!shaderObj) {
            return false;
        }
        else {
			temp_sle_Shader = shaderObj;
            return true;
        }
    }
    catch (...) {
        // 异常处理
        return false;
    }
}