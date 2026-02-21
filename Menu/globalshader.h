#pragma once
#include "engine.h"
#include "base.h"
#include <vector>
#include <string>

class globalshader
{
public:
	static Shader* temp_sle_Shader;
	static vector<string> shaderNames;
	static vector<string> cachedShaderNames;  // 缓存上一次的着色器名称列表
	static bool init();
	static bool get_shader_by_name(const string& name);
};

