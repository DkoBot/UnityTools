#pragma once
#include "engine.h"
#include "base.h"
#include <vector>
#include <string>


// 着色器属性结构体
struct ShaderPropertyInfo {
    int type;           // ShaderPropertyType 类型
    string name;   // 属性名称
    float floatValue;   // Float/Range 类型的值
    int intValue;       // Int 类型的值
    Vector4 vectorValue; // Vector 类型的值
    Color colorValue;   // Color 类型的值
	string textureValue; // Texture 类型的值（纹理名称）
    
    ShaderPropertyInfo() : type(0), floatValue(0.0f), intValue(0) {
        vectorValue.x = vectorValue.y = vectorValue.z = vectorValue.w = 0.0f;
        colorValue.r = colorValue.g = colorValue.b = colorValue.a = 0.0f;
    }
};
class globalshader
{
public:
	static Shader* temp_sle_Shader;
	static Shader* cachedSelectedShader;  // 缓存当前选中的着色器指针
	static vector<string> shaderNames;
	static vector<string> cachedShaderNames;  // 缓存上一次的着色器名称列表
	static vector<ShaderPropertyInfo> cachedProperties;  // 缓存当前着色器的属性列表
	static bool init();
	static bool get_shader_by_name(const string& name);
	static void GetShader();
	static vector<ShaderPropertyInfo> GetShaderProperties();
};

