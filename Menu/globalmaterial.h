#pragma once
#include "engine.h"
#include "base.h"
#include <vector>
#include <string>
#include <map>


// 着色器属性结构体
struct MaterialPropertyInfo {
    int type;           // ShaderPropertyType 类型
    string name;   // 属性名称
    float floatValue;   // Float/Range 类型的值
    int intValue;       // Int 类型的值
    Vector4 vectorValue; // Vector 类型的值
    Color colorValue;   // Color 类型的值
    string textureValue; // Texture 类型的值（纹理名称）

    MaterialPropertyInfo() : type(0), floatValue(0.0f), intValue(0) {
        vectorValue.x = vectorValue.y = vectorValue.z = vectorValue.w = 0.0f;
        colorValue.r = colorValue.g = colorValue.b = colorValue.a = 0.0f;
    }
};

class globalmaterial {
public:
	static std::map<string, void*> materialNames;  // 材质名称到实例地址的字典
	static std::map<string, void*> cachedMaterialNames;  // 缓存上一次的材质字典
	static Material* temp_material;  // 临时材质指针
	static std::vector<MaterialPropertyInfo> cachedProperties;  // 缓存的属性列表
	static Material* cachedSelectedMaterial;  // 缓存选中的材质
	static bool init();
	static bool get_material_by_name(const string& name);
    static std::vector<MaterialPropertyInfo> GetMaterialProperties();
	static void GetMaterial();
};

