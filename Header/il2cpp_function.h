DO_FUNC(Il2CppArray*, FindObjectsOfType, (void*), "UnityEngine.CoreModule", "UnityEngine", "Object", "Object[] FindObjectsOfType(Type type);");
DO_FUNC(Il2CppArray*, FindObjectsOfTypeAll, (void*), "UnityEngine.CoreModule", "UnityEngine", "Resources", "Object[] FindObjectsOfTypeAll(Type type);");
DO_FUNC(Il2CppArray*, FindObjectsByTypeBool , (void*,bool), "UnityEngine.CoreModule", "UnityEngine", "Object", "Object[] FindObjectsOfType(Type type,Boolean includeInactive);");
//DO_FUNC(Il2CppArray*, FindGameObjectsWithTag, (Il2CppString*), "UnityEngine.CoreModule", "UnityEngine", "GameObject", "GameObject[] FindGameObjectsWithTag(String tag);");
DO_FUNC(void*, GetType, (Il2CppString*), "mscorlib", "System", "Type", "Type GetType(String typeName);");
DO_FUNC(void*, PtrToStringAnsi, (const char*), "mscorlib", "System.Runtime.InteropServices", "Marshal", "String PtrToStringAnsi(IntPtr ptr);");
DO_FUNC(Camera*, GetMainCamera, (), "UnityEngine.CoreModule", "UnityEngine", "Camera", "Camera get_main();");
DO_FUNC(void*, GameObject_Find, (Il2CppString*), "UnityEngine.CoreModule", "UnityEngine", "GameObject", "GameObject Find(String name);");

DO_FUNC(Transform*, GetTransform, (GameObject*), "UnityEngine.CoreModule", "UnityEngine", "GameObject", "Transform get_transform();");
DO_FUNC(void, SetActive, (GameObject*,bool), "UnityEngine.CoreModule", "UnityEngine", "GameObject", "Void SetActive(Boolean value);");
DO_FUNC(Transform*, GetTransform_Component, (void*), "UnityEngine.CoreModule", "UnityEngine", "Component", "Transform get_transform();");
DO_FUNC(Vector3, GetPosition, (Transform*), "UnityEngine.CoreModule", "UnityEngine", "Transform", "Vector3 get_position();");
DO_FUNC(Vector3, GetEulerAngles, (Transform*), "UnityEngine.CoreModule", "UnityEngine", "Transform", "Vector3 get_eulerAngles();");
DO_FUNC(GameObject*, GetGameObject, (void*), "UnityEngine.CoreModule", "UnityEngine", "Component", "GameObject get_gameObject();");
DO_FUNC(Il2CppString*, Gettag, (void*), "UnityEngine.CoreModule", "UnityEngine", "Component", "String get_tag();");
DO_FUNC(int, GetChildCount, (Transform*), "UnityEngine.CoreModule", "UnityEngine", "Transform", "Int32 get_childCount();");
DO_FUNC(Transform*, GetChild, (Transform*, int), "UnityEngine.CoreModule", "UnityEngine", "Transform", "Transform GetChild(Int32 index);");
DO_FUNC(Vector3, GetForward, (Transform*), "UnityEngine.CoreModule", "UnityEngine", "Transform", "Vector3 get_forward();");
DO_FUNC(Shader*, FindShader, (Il2CppString*), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Shader Find(String name);");


DO_FUNC(Il2CppString*, GetShaderPropertyName, (void*, int), "UnityEngine.CoreModule", "UnityEngine", "Shader", "String GetPropertyName(Int32 propertyIndex);");
DO_FUNC(Il2CppString*, GetShaderPropertyDescription, (void*, int), "UnityEngine.CoreModule", "UnityEngine", "Shader", "String GetPropertyDescription(Int32 propertyIndex);");
DO_FUNC(ShaderPropertyType, GetShaderPropertyType, (void*,int), "UnityEngine.CoreModule", "UnityEngine", "Shader", "ShaderPropertyType GetPropertyType(Int32 propertyIndex);");
DO_FUNC(int, GetShaderPropertyCount, (void*), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Int32 GetPropertyCount();");
DO_FUNC(void, SetGlobalColor, (void*,Il2CppString*,Color), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Void SetGlobalColor(String name,Color value);");
DO_FUNC(void, SetGlobalMatrix, (void*, Il2CppString*,Matrix4x4), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Void SetGlobalMatrix(String name,Matrix4x4 value);");
DO_FUNC(void, SetGlobalVector, (void*, Il2CppString*,Vector4), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Void SetGlobalVector(String name,Vector4 value);");
DO_FUNC(void, SetGlobalFloat, (void*, Il2CppString*,float), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Void SetGlobalFloat(String name,Single value);");
DO_FUNC(void, SetGlobalInt, (void*, Il2CppString*,int), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Void SetGlobalInt(String name,Int32 value);");
DO_FUNC(float, GetPropertyDefaultFloatValue, (void*, int), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Single GetPropertyDefaultFloatValue(Int32 propertyIndex);");
DO_FUNC(int, GetPropertyDefaultIntValue, (void*, int), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Int32 GetPropertyDefaultIntValue(Int32 propertyIndex);");
DO_FUNC(Vector4, GetPropertyDefaultVectorValue, (void*, int), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Vector4 GetPropertyDefaultVectorValue(Int32 propertyIndex);");
DO_FUNC(Il2CppString*, GetPropertyTextureDefaultName, (void*, int), "UnityEngine.CoreModule", "UnityEngine", "Shader", "String GetPropertyTextureDefaultName(Int32 propertyIndex);");
DO_FUNC(TextureDimension, GetPropertyTextureDimension, (void*, int), "UnityEngine.CoreModule", "UnityEngine", "Shader", "TextureDimension GetPropertyTextureDimension(Int32 propertyIndex);");
DO_FUNC(Color*, GetGlobalColor, (void*, Il2CppString*), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Color GetGlobalColor(String name);");
DO_FUNC(void, SetMaximumLOD, (void*, int), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Void set_maximumLOD(Int32 value);");
DO_FUNC(void, SetGlobalMaximumLOD, (int), "UnityEngine.CoreModule", "UnityEngine", "QualitySettings", "Void set_maximumLODLevel(Int32 value);");
DO_FUNC(int, GetPassCount, (void*), "UnityEngine.CoreModule", "UnityEngine", "Shader", "Int32 get_passCount();");
DO_FUNC(Il2CppString*, GetUnityVersion, (), "UnityEngine.CoreModule", "UnityEngine", "Application", "String get_unityVersion();");


DO_FUNC(void, Setcolor_Material, (void*, Il2CppString*, Color*), "UnityEngine.CoreModule", "UnityEngine", "Material", "Void SetColor(String name,Color value);");
DO_FUNC(void, SetFloat_Material, (void*, Il2CppString*, float), "UnityEngine.CoreModule", "UnityEngine", "Material", "Void SetFloat(String name,Single value);");
DO_FUNC(void, SetInt_Material, (void*, Il2CppString*, int), "UnityEngine.CoreModule", "UnityEngine", "Material", "Void SetInt(String name,Int32 value);");
DO_FUNC(void, SetVector_Material, (void*, Il2CppString*, Vector4), "UnityEngine.CoreModule", "UnityEngine", "Material", "Void SetVector(String name,Vector4 value);");
DO_FUNC(Il2CppArray*, Get_PropertyNames, (void*, MaterialPropertyType), "UnityEngine.CoreModule", "UnityEngine", "Material", "String[] GetPropertyNames(MaterialPropertyType type);");
DO_FUNC(int, GetInt_Material, (void*, Il2CppString*), "UnityEngine.CoreModule", "UnityEngine", "Material", "Int32 GetInt(String name);");
DO_FUNC(Color, Getcolor_Material, (void*, Il2CppString*), "UnityEngine.CoreModule", "UnityEngine", "Material", "Color GetColor(String name);");
DO_FUNC(float, GetFloat_Material, (void*, Il2CppString*), "UnityEngine.CoreModule", "UnityEngine", "Material", "Single GetFloat(String name);");
DO_FUNC(Vector4, GetVector_Material, (void*, Il2CppString*), "UnityEngine.CoreModule", "UnityEngine", "Material", "Vector4 GetVector(String name);");
DO_FUNC(Texture*, GetTexture_Material, (void*, Il2CppString*), "UnityEngine.CoreModule", "UnityEngine", "Material", "Texture GetTexture(String name);");
DO_FUNC(Il2CppArray*, Get_ShaderKeywords, (void*), "UnityEngine.CoreModule", "UnityEngine", "Material", "String[] GetShaderKeywords();");
DO_FUNC(Il2CppArray*, Get_TexturePropertyNames, (void*), "UnityEngine.CoreModule", "UnityEngine", "Material", "String[] GetTexturePropertyNames();");