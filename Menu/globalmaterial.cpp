#include "globalmaterial.h"

std::map<string, void*> globalmaterial::materialNames;
std::map<string, void*> globalmaterial::cachedMaterialNames;
Material* globalmaterial::temp_material = nullptr;
std::vector<MaterialPropertyInfo> globalmaterial::cachedProperties;
Material* globalmaterial::cachedSelectedMaterial = nullptr;


bool globalmaterial::init() {
    cachedMaterialNames = materialNames;
    materialNames.clear();

    try {
        il2cpp_thread_attach(il2cpp_domain_get());

        // 获取 Material 类型
        string fullTypeName = "UnityEngine.Material, UnityEngine.CoreModule";
        Il2CppString* typeName = Engine::create_il2cpp_string(wstring(fullTypeName.begin(), fullTypeName.end()).c_str());
        void* materialType = GetType(typeName);

        if (!materialType) {
            return false;
        }

        // 使用 FindObjectsOfTypeAll 获取所有 Material 对象
        Il2CppArray* materialArray = (Il2CppArray*)FindObjectsOfTypeAll(materialType);

        if (!materialArray || materialArray == (Il2CppArray*)0xCCCCCCCCCCCCCCCC || materialArray->max_length == 0 || !materialArray->vector) {
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

        // 用于统计同名材质的数量
        std::map<string, int> nameCount;

        // 遍历所有 Material 对象并获取名称
        for (int i = 0; i < materialArray->max_length; ++i) {
            void* materialObj = ((void**)materialArray->vector)[i];
            if (!materialObj) continue;

            // 调用 get_name() 方法获取名称
            Il2CppObject* exc = nullptr;
            Il2CppString* nameStr = (Il2CppString*)il2cpp_runtime_invoke(getNameMethod, materialObj, nullptr, &exc);

            if (nameStr && !exc) {
                string materialName = Engine::il2cppStringToStdString(nameStr);
                if (!materialName.empty()) {
                    // 检查是否有重复
                    string finalName = materialName;
                    if (nameCount.find(materialName) != nameCount.end()) {
                        // 已有同名材质，增加后缀
                        int count = nameCount[materialName];
                        char suffix[32];
                        sprintf_s(suffix, sizeof(suffix), " (%d)", count);
                        finalName = materialName + suffix;
                    }
                    nameCount[materialName]++;
                    materialNames[finalName] = materialObj;
                }
            }
        }
        return true;
    }
    catch (...) {
        return false;
    }
}
bool globalmaterial::get_material_by_name(const string& name) {
    temp_material = nullptr;
    cachedProperties.clear();
    cachedSelectedMaterial = nullptr;
    auto it = materialNames.find(name);
    if (it != materialNames.end()) {
        temp_material = (Material*)it->second;
        if (*(void**)((uintptr_t)temp_material + 0x10) != nullptr) {
            return true;
        }
    }
    return false;
}

vector<MaterialPropertyInfo> globalmaterial::GetMaterialProperties() {
    vector<MaterialPropertyInfo> properties;
    Material* material = temp_material;
    if (material) {
        Il2CppArray* keywords = material->GetShaderKeywords();
        Il2CppArray* textureProps = material->GetTexturePropertyNames();
        Il2CppArray* floatProps = material->GetPropertyNames(MaterialPropertyType::MaterialPropertyType_Float);
        Il2CppArray* intProps = material->GetPropertyNames(MaterialPropertyType::MaterialPropertyType_Int);
        Il2CppArray* vectorProps = material->GetPropertyNames(MaterialPropertyType::MaterialPropertyType_Vector);
        Il2CppArray* matrixProps = material->GetPropertyNames(MaterialPropertyType::MaterialPropertyType_Matrix);

        // Keywords
        if (keywords) {
            for (int i = 0; i < keywords->max_length; i++) {
                Il2CppString* keyword = (Il2CppString*)keywords->vector[i];
                if (keyword) {
                    MaterialPropertyInfo prop;
                    prop.type = ShaderType_Float;  // 用 Float 作为 Keywords 类型标记
                    prop.name = Engine::il2CppStringToUtf8(keyword);
                    prop.floatValue = 1.0f;  // 用 floatValue 表示开启状态
                    properties.push_back(prop);
                }
            }
        }
        // Texture Properties
        if (textureProps) {
            for (int i = 0; i < textureProps->max_length; i++) {
                Il2CppString* propName = (Il2CppString*)textureProps->vector[i];
                if (propName) {
                    string name = Engine::il2CppStringToUtf8(propName);
                    Texture* tex = material->GetTexture(propName);
                    void* addr = (void*)(tex ? *((void**)tex) : nullptr);
                    MaterialPropertyInfo prop;
                    prop.type = ShaderType_Texture;
                    prop.name = name;
                    char buf[64];
                    sprintf_s(buf, sizeof(buf), "%p", addr);
                    prop.textureValue = buf;
                    properties.push_back(prop);
                }
            }
        }
        // Float Properties
        if (floatProps) {
            for (int i = 0; i < floatProps->max_length; i++) {
                Il2CppString* propName = (Il2CppString*)floatProps->vector[i];
                if (propName) {
                    string name = Engine::il2CppStringToUtf8(propName);
                    float val = material->GetFloat(propName);
                    MaterialPropertyInfo prop;
                    prop.type = ShaderType_Float;
                    prop.name = name;
                    prop.floatValue = val;
                    properties.push_back(prop);
                }
            }
        }
        // Int Properties
        if (intProps) {
            for (int i = 0; i < intProps->max_length; i++) {
                Il2CppString* propName = (Il2CppString*)intProps->vector[i];
                if (propName) {
                    string name = Engine::il2CppStringToUtf8(propName);
                    int val = material->GetInt(propName);
                    MaterialPropertyInfo prop;
                    prop.type = ShaderType_Int;
                    prop.name = name;
                    prop.intValue = val;
                    properties.push_back(prop);
                }
            }
        }
        // Vector Properties
        if (vectorProps) {
            for (int i = 0; i < vectorProps->max_length; i++) {
                Il2CppString* propName = (Il2CppString*)vectorProps->vector[i];
                if (propName) {
                    string name = Engine::il2CppStringToUtf8(propName);
                    Vector4 val = material->GetVector(propName);
                    MaterialPropertyInfo prop;
                    prop.type = ShaderType_Vector;
                    prop.name = name;
                    prop.vectorValue = val;
                    properties.push_back(prop);
                }
            }
        }
    }
    return properties;
}

void globalmaterial::GetMaterial() {
    GetMaterialProperties();
}


