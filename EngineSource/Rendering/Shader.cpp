#define _CRT_SECURE_NO_WARNINGS
#include "Shader.h"
#include <fstream>
#include <iostream>
#include <Utility/stb_image.h>
#include <sstream>
#include <Engine/Log.h>
#include <Importers/Build/Pack.h>
#include <GL/glew.h>


extern const bool IsInEditor;
extern const bool EngineDebug;

Shader::Shader(const char* VertexShaderFilename, const char* FragmentShaderFilename, const char* GeometryShader)
{

	ShaderID = CreateShader(VertexShaderFilename, FragmentShaderFilename, GeometryShader);
	VertexFileName = VertexShaderFilename;
	FragmetFileName = FragmentShaderFilename;
}

Shader::~Shader()
{
	glDeleteProgram(ShaderID);
}

void Shader::Bind()
{
	glUseProgram(ShaderID);
}

void Shader::Unbind()
{
	glUseProgram(0);
}


void Shader::Recompile()
{
	throw "not implemented";
	glDeleteProgram(ShaderID);
	CreateShader(VertexFileName.c_str(), FragmetFileName.c_str(), nullptr);
}

GLuint Shader::Compile(std::string ShaderCode, unsigned int Type)
{
	GLuint id = glCreateShader(Type);
	const char* src = ShaderCode.c_str();
	glShaderSource(id, 1, &src, 0);

	glCompileShader(id);

	int Result;

	glGetShaderiv(id, GL_COMPILE_STATUS, &Result);
	if (Result != GL_TRUE)
	{
		int length = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = new char[length];
		glGetShaderInfoLog(id, length, &length, message);
		Log::CreateNewLogMessage(std::string("Warning: OpenGL Shader Compile Error : ").append(message));
		delete[] message;
		return 0;
	}
	return id;
}

std::string Shader::parse(const char* Filename)
{
	FILE* File;
	File = fopen(Filename, "rb");
	if (File == nullptr)
	{
		Log::CreateNewLogMessage("File " + std::string(Filename) + " could not be found.");
		return std::string("");
	}
	std::string ShaderCode;
	fseek(File, 0, SEEK_END);
	size_t FileSize = ftell(File);
	rewind(File);
	ShaderCode.resize(FileSize);
	fread(&ShaderCode[0], 1, FileSize, File);
	fclose(File);


	return ShaderCode;
}

GLuint Shader::CreateShader(const char* VertexShader, const char* FragmentShader, const char* GeometryShader)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string sharedCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream fSharedFile;
	std::ifstream gShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fSharedFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	if (EngineDebug || IsInEditor)
	{
		// open files
		vShaderFile.open(VertexShader);
		fShaderFile.open(FragmentShader);
		fSharedFile.open("Shaders/shared.frag");
		std::stringstream vShaderStream, fShaderStream, fSharedStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		fSharedStream << fSharedFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		fSharedFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		sharedCode = fSharedStream.str();
		// if geometry shader path is present, also load a geometry shader
		if (GeometryShader != nullptr)
		{
			gShaderFile.open(GeometryShader);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	else
	{
		vertexCode = Pack::GetFile(VertexShader);
		fragmentCode = Pack::GetFile(FragmentShader);
		sharedCode = Pack::GetFile("shared.frag");
		// if geometry shader path is present, also load a geometry shader
		if (GeometryShader != nullptr)
		{
			geometryCode = Pack::GetFile(GeometryShader);
		}
	}
	if(fragmentCode.substr(0, 3) == "//!")
	fragmentCode = sharedCode + fragmentCode;
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX", VertexShader);
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT", fShaderCode);
	// fragment common functions Shader
	// if geometry shader is given, compile geometry shader
	unsigned int geometry;
	if (GeometryShader != nullptr)
	{
		const char* gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);

		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY", GeometryShader);
	}

	// shader Program
	ShaderID = glCreateProgram();
	glAttachShader(ShaderID, vertex);
	glAttachShader(ShaderID, fragment);
	if (GeometryShader != nullptr)
		glAttachShader(ShaderID, geometry);
	glLinkProgram(ShaderID);
	checkCompileErrors(ShaderID, "PROGRAM", (VertexShader + std::string("-") + FragmentShader));
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (GeometryShader != nullptr)
		glDeleteShader(geometry);
	return ShaderID;
}
void Shader::checkCompileErrors(unsigned int shader, std::string type, std::string ShaderName)
{
	GLint success;
	GLchar infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			std::cout << ShaderName << "::";

			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n INFO LOG: \n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			std::cin.get();

		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			std::cout << ShaderName << "::";

			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n INFO LOG: \n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			std::cin.get();

		}
	}
}