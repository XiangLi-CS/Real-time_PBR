/******************************************************************************
Class:Shader
Implements:
Author:Rich Davison	 <richard-gordon.davison@newcastle.ac.uk>
Description:VERY simple class to encapsulate GLSL shader loading, linking,
and binding. Useful additions to this class would be overloaded functions to
replace the glUniformxx functions in external code, and possibly a map to store
uniform names and their resulting bindings. 

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "OGLRenderer.h"

enum ShaderStage {
	SHADER_VERTEX,
	SHADER_FRAGMENT,
	SHADER_GEOMETRY,
	SHADER_DOMAIN,
	SHADER_HULL,
	SHADER_MAX
};

class Shader	{
public:
	Shader(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "");
	~Shader(void);

	GLuint  GetProgram() { return programID;}
	
	void	Reload(bool deleteOld = true);

	bool	LoadSuccess() {
		return shaderValid[0] == GL_TRUE && programValid == GL_TRUE;
	}

	void SetInt(const std::string& name, const int& val);
	void SetFloat(const std::string& name, const float& val);
	void SetMat4(const std::string& name, const Matrix4& val, bool transposed = false);
	void SetVector2(const std::string& name, const Vector2& val);
	void SetVector3(const std::string& name, const Vector3& val);
	void SetVector4(const std::string& name, const Vector4& val);
	void SetTexture(const std::string& name, const unsigned int& texID, const int& texSlot);
	void SetTextureCubeMap(const std::string& name, const unsigned int& texID, const int& texSlot);

	void Bind();

	static void ReloadAllShaders();
	static void	PrintCompileLog(GLuint object);
	static void	PrintLinkLog(GLuint program);

protected:
	void	DeleteIDs();

	bool	LoadShaderFile(const  std::string& from, std::string &into);
	void	GenerateShaderObject(unsigned int i);
	void	SetDefaultAttributes();
	void	LinkProgram();

	GLuint	programID;
	GLuint	objectIDs[SHADER_MAX];
	GLint	programValid;
	GLint	shaderValid[SHADER_MAX];

	std::string  shaderFiles[SHADER_MAX];

	static std::vector<Shader*> allShaders;
};

