#pragma once
#include "base.h"

// 前向声明
struct Il2CppString;

class TableClass {
public:
	char* AssName;
};

class MonoVTable {
public:
	void* MoveAss;
	char pad_010[0x10];     // 修复：填充到0x10，原写法有歧义
	void* ClassName;         // 0x10
	char pad_018[0x8];      // 修复：从0x18到0x18？应该是0x10+8=0x18
	void* NameSpaceName;     // 0x18
};

struct D3DXMATRIX
{
	FLOAT _11; FLOAT _12; FLOAT _13; FLOAT _14;
	FLOAT _21; FLOAT _22; FLOAT _23; FLOAT _24;
	FLOAT _31; FLOAT _32; FLOAT _33; FLOAT _34;
	FLOAT _41; FLOAT _42; FLOAT _43; FLOAT _44;
};
class Vector2 {
public:
	float x, y;
	Vector2() {
		RtlZeroMemory(this, sizeof(Vector2));
	}
};
class Vector3 {
public:
	float x, y, z;
	Vector3() {
		RtlZeroMemory(this, sizeof(Vector3));
	}
	Vector3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
	Vector3 operator-(Vector3& v) {
		return Vector3(x - v.x, y - v.y, z - v.z);
	}
	Vector3 Normalized()
	{
		float length = std::sqrt(x * x + y * y + z * z);

		if (length > 0.0000099999997) {
			float invLength = 1.0f / length;
			return { x * invLength, y * invLength, z * invLength };
		}

		return { 0.0f, 0.0f, 0.0f };
	}
	float GetDistance() {
		return std::sqrt(x * x + y * y + z * z);
	}
};

class Vector4 {
	public:
	float x, y, z, w;
	Vector4() {
		RtlZeroMemory(this, sizeof(Vector4));
	}
};
class Matrix4x4 {
public:
	float m00, m01, m02, m03;
	float m10, m11, m12, m13;
	float m20, m21, m22, m23;
	float m30, m31, m32, m33;
	
	Matrix4x4() {
		RtlZeroMemory(this, sizeof(Matrix4x4));
		// 初始化为单位矩阵
		m00 = m11 = m22 = m33 = 1.0f;
	}
	
	// 通过数组索引访问（行优先）
	float& operator[](int index) {
		return *(&m00 + index);
	}
	const float& operator[](int index) const {
		return *(&m00 + index);
	}
	
	// 通过行列索引访问
	float& operator()(int row, int col) {
		return *(&m00 + row * 4 + col);
	}
	const float& operator()(int row, int col) const {
		return *(&m00 + row * 4 + col);
	}
};
class Color {
public:
	float r, g, b, a; // RGBA 四个分量，范围通常是 0.0 - 1.0
	Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
	Color(float R, float G, float B, float A = 1.0f) : r(R), g(G), b(B), a(A) {}
};
enum FindObjectsSortMode : int32_t
{
	None = 0,
	InstanceID = 1,
	Name = 2,
	HierarchyOrder = 3
};
class Il2CppClass {
public:
	char pad_000[0x10];
	const char* ClassName;
	const char* Namespace;
};
struct Il2CppObject {          // 所有托管对象头
	void* klass;
	void* monitor;
};
struct Il2CppString : Il2CppObject {
	int32_t length;
	wchar_t chars[1];          // 柔性数组
};
struct Il2CppArray : Il2CppObject {
	void* bounds;
	int32_t  max_length;
	void* vector[1];        // 元素从这里开始
};
struct System_Guid {
	uint32_t _a;   // 0x00
	uint16_t _b;   // 0x04
	uint16_t _c;   // 0x06
	uint8_t  _d;   // 0x08
	uint8_t  _e;   // 0x09
	uint8_t  _f;   // 0x0A
	uint8_t  _g;   // 0x0B
	uint8_t  _h;   // 0x0C
	uint8_t  _i;   // 0x0D
	uint8_t  _j;   // 0x0E
	uint8_t  _k;   // 0x0F

	// 重载==运算符，用于比较两个GUID是否相等
	bool operator==(const System_Guid& other) const {
		return (_a == other._a) &&
			   (_b == other._b) &&
			   (_c == other._c) &&
			   (_d == other._d) &&
			   (_e == other._e) &&
			   (_f == other._f) &&
			   (_g == other._g) &&
			   (_h == other._h) &&
			   (_i == other._i) &&
			   (_j == other._j) &&
			   (_k == other._k);
	}
};
class Unity_Array {
public:
	char pad_0000[0x18];
	size_t Count;
	void* Objects[65536];
};
class Object {
public:
	Il2CppClass* pClass;
};
class Camera {
public:
	bool WorldToScreen(Vector3 position, Vector2& Point);
};
class Component :public Object {
public:
	string get_tag();
};
class Transform {
public:
	Vector3 GetPosition();
	int GetChildCount();
	Transform* GetChild(int index);
	Vector3 GetForward();
};
class GameObject :public Object {
public:
	Transform* GetTransform();
	Component* GetComponent(string Type);
	Component* GetComponentInChildren(string Type);
	void SetActive(bool offoron);
};
class Shader {
public:
	/*
	chamsMaterial.SetInt("_Cull", 0);
	chamsMaterial.SetInt("_ZWrite", 0);
	chamsMaterial.SetInt("_ZTest", 8);
	chamsMaterial.SetColor("_Color", curChamsColour);
	*/

	static void SetGlobalColor(string name, Color value);
	static void SetGlobalInt(string name, INT32 value);
	static void SetGlobalFloat(string name, float value);
	static void SetGlobalVector(string name, Vector4 value);
	static void SetGlobalMatrix(string name, Matrix4x4 value);
	void set_maximumLOD(INT32 value);
	int get_passCount();

};
class Material {
public:
	// No fields found

	// Methods
	void CreateWithShader(Material self, Shader shader);
	void CreateWithMaterial(Material self, Material source);
	void CreateWithString(Material self);
	Shader get_shader();
	void set_shader(Shader value);
	Color get_color();
	void set_color(Color value);
	Vector2 get_mainTextureOffset();
	void set_mainTextureOffset(Vector2 value);
	Vector2 get_mainTextureScale();
	void set_mainTextureScale(Vector2 value);
	bool HasProperty(int nameID);
	bool HasProperty(Il2CppString* name);
	bool HasIntImpl(int name);
	bool HasInteger(int nameID);
	bool HasVectorImpl(int name);
	bool HasVector(int nameID);
	bool HasColor(int nameID);
	int get_renderQueue();
	void set_renderQueue(int value);
	int get_rawRenderQueue();
	void EnableKeyword(Il2CppString* keyword);
	void DisableKeyword(Il2CppString* keyword);
	bool IsKeywordEnabled(Il2CppString* keyword);
	bool get_enableInstancing();
	void set_enableInstancing(bool value);
	int get_passCount();
	void SetShaderPassEnabled(Il2CppString* passName, bool enabled);
	int FindPass(Il2CppString* passName);
	void SetOverrideTag(Il2CppString* tag, Il2CppString* val);
	Il2CppString* GetTagImpl(Il2CppString* tag, bool currentSubShaderOnly, Il2CppString* defaultValue);
	Il2CppString* GetTag(Il2CppString* tag, bool searchFallbacks);
	void Lerp(Material start, Material end, float t);
	bool SetPass(int pass);
	void CopyPropertiesFromMaterial(Material mat);
	void CopyMatchingPropertiesFromMaterial(Material mat);
	int ComputeCRC();
	void SetIntImpl(int name, int value);
	void SetFloatImpl(int name, float value);
	void SetColorImpl(int name, Color value);
	void SetMatrixImpl(int name, Matrix4x4 value);
	float GetFloatImpl(int name);
	Color GetColorImpl(int name);
	Matrix4x4 GetMatrixImpl(int name);
	int GetFloatArrayCountImpl(int name);
	int GetVectorArrayCountImpl(int name);
	int GetColorArrayCountImpl(int name);
	void SetTextureOffsetImpl(int name, Vector2 offset);
	void SetTextureScaleImpl(int name, Vector2 scale);
	void SetInt(Il2CppString* name, int value);
	void SetInt(int nameID, int value);
	void SetFloat(Il2CppString* name, float value);
	void SetFloat(int nameID, float value);
	void SetInteger(int nameID, int value);
	void SetColor(Il2CppString* name, Color value);
	void SetColor(int nameID, Color value);
	void SetMatrix(Il2CppString* name, Matrix4x4 value);
	void SetMatrix(int nameID, Matrix4x4 value);
	int GetInt(int nameID);
	float GetFloat(Il2CppString* name);
	float GetFloat(int nameID);
	Color GetColor(Il2CppString* name);
	Color GetColor(int nameID);
	Matrix4x4 GetMatrix(Il2CppString* name);
	void SetTextureOffset(Il2CppString* name, Vector2 value);
	void SetTextureOffset(int nameID, Vector2 value);
	void SetTextureScale(Il2CppString* name, Vector2 value);
	void SetTextureScale(int nameID, Vector2 value);
	Vector2 GetTextureOffset(Il2CppString* name);
	Vector2 GetTextureOffset(int nameID);
	Vector2 GetTextureScale(Il2CppString* name);
	Vector2 GetTextureScale(int nameID);
	void SetColorImpl_Injected(int name, Color& value);
	void SetMatrixImpl_Injected(int name, Matrix4x4& value);
	void GetColorImpl_Injected(int name, Color& ret);
	void GetMatrixImpl_Injected(int name, Matrix4x4& ret);
	void GetTextureScaleAndOffsetImpl_Injected(int name, Vector4& ret);
	void SetTextureOffsetImpl_Injected(int name,Vector2& offset);
	void SetTextureScaleImpl_Injected(int name, Vector2& scale);
	Il2CppArray* GetTexturePropertyNames();
};
