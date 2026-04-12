#include "globalshader.h"
#include "../Misc/ColorInfo/InfoMesseng.h"
#include <cstring>
#include <cstdio>

vector<string> globalshader::shaderNames;
vector<string> globalshader::cachedShaderNames;
Shader* globalshader::temp_sle_Shader;
Shader* globalshader::cachedSelectedShader = nullptr;
vector<ShaderPropertyInfo> globalshader::cachedProperties;

bool globalshader::init() {
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
                    // 去重检查
                    bool isDuplicate = false;
                    for (const auto& existing : shaderNames) {
                        if (existing == shaderName) {
                            isDuplicate = true;
                            break;
                        }
                    }
                    if (!isDuplicate) {
                        shaderNames.push_back(shaderName);
                    }
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
vector<ShaderPropertyInfo> globalshader::GetShaderProperties() {
    vector<ShaderPropertyInfo> properties;
    Shader* shader = temp_sle_Shader;
    
    if (shader) {
        if (cachedSelectedShader != shader) {
            cachedSelectedShader = shader;
            cachedProperties.clear();
        }
        int propertyCount = shader->GetPropertyCount();
        char shaderInfo[256];
        sprintf_s(shaderInfo, "Shader: 0x%p, Property Count: %d", shader, propertyCount);
        InfoMesseng::ColorPrint("SUCCESS", shaderInfo, 0);

        for (int i = 0; i < propertyCount; ++i) {
            ShaderPropertyInfo info;
            info.type = (int)shader->GetPropertyType(i);
            Il2CppString* description = shader->GetPropertyDescription(i);
			info.name = description ? Engine::il2cppStringToStdString(description) : "Unknown";
            string texturetype = "";
            string texturevalue = "";
            switch (info.type) {
                case ShaderType_Float:
                    info.floatValue = shader->GetFloat(i);
                    break;
                case ShaderType_Range:
                    info.floatValue = shader->GetFloat(i);
                    break;
                case ShaderType_Int:
                    info.intValue = shader->GetInt(i);
                    break;
                case ShaderType_Vector:
                    info.vectorValue = shader->GetVector(i);
                    break;
                case ShaderType_Color:
                    info.colorValue = shader->GetColor(i);
                    break;
                case ShaderType_Texture:
                    switch (shader->GetTextureDimension(i))
                    {
                       case 1:
                       texturetype = "| 1D";
                       break;
                       case 2:
                       texturetype = "| 2D";
                       break;
                       case 3:
                       texturetype = "| 3D";
                       break;
                       case 4:
                       texturetype = "| 4D";
                       break;
                       default:
                       texturetype = " | Unknown";
                       break;
                    }

                    texturevalue = shader->GetTexture(i);
                    if (texturevalue.empty()) {
                        texturevalue = "None";
					}
					info.textureValue = string(texturevalue) + texturetype;
					break;
                default:
                    break;
            }
            cachedProperties.push_back(info);
            properties.push_back(info);
        }
    }
    
    return properties;
}
void globalshader::GetShader() {
    GetShaderProperties();
}