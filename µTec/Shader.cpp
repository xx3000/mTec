// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#include "Shader.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>

using namespace µTec;

GLuint BaseShader::LoadShader( const char * file_path, unsigned int type)
{
	// Create the shaders

	GLuint subShaderID = glCreateShader(type);
	//GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string shaderCode;
	std::ifstream shaderStream(file_path, std::ios::in);
	if(shaderStream.is_open())
	{
		std::string Line = "";
		while(getline(shaderStream, Line))
			shaderCode += "\n" + Line;
		shaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Shader
	printf("Compiling shader : %s\n", file_path);
	char const * sourcePointer = shaderCode.c_str();
	glShaderSource(subShaderID, 1, &sourcePointer , NULL);
	glCompileShader(subShaderID);

	// Check Shader
	glGetShaderiv(subShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(subShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> shaderErrorMessage(InfoLogLength);
	if(shaderErrorMessage.size() > 0) 
		{
	glGetShaderInfoLog(subShaderID, InfoLogLength, NULL, &shaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &shaderErrorMessage[0]);
	}

	return subShaderID;
}



BaseShader::BaseShader( const char * vertex_file_path,const char * fragment_file_path ) : m_fragment_shader_path(fragment_file_path), m_vertex_shader_path(vertex_file_path)
{
	std::vector<GLuint> subShaders;
	subShaders.push_back(LoadShader(vertex_file_path,GL_VERTEX_SHADER));
	subShaders.push_back(LoadShader(fragment_file_path,GL_FRAGMENT_SHADER));
	shaderID=linkShaders(subShaders);
}

BaseShader::BaseShader( const char * vertex_file_path,const char * geometry_file_path,const char * fragment_file_path ): m_fragment_shader_path(fragment_file_path), m_vertex_shader_path(vertex_file_path), m_geometry_shader_path(geometry_file_path)
{
	std::vector<GLuint> subShaders;
	subShaders.push_back(LoadShader(vertex_file_path,GL_VERTEX_SHADER));
	subShaders.push_back(LoadShader(geometry_file_path,GL_GEOMETRY_SHADER));
	subShaders.push_back(LoadShader(fragment_file_path,GL_FRAGMENT_SHADER));
	shaderID=linkShaders(subShaders);
}

BaseShader::~BaseShader( void )
{

}

GLuint BaseShader::linkShaders( std::vector<GLuint> subShaderID )
{
	GLint Result = GL_FALSE;
	int InfoLogLength;
	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	for(unsigned i=0; i< subShaderID.size();++i)
	glAttachShader(ProgramID, subShaderID[i]);

	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( std::max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	for(unsigned i=0; i< subShaderID.size();++i)
	glDeleteShader(subShaderID[i]);
	
	return ProgramID;
}

µTec::ComputeShader::ComputeShader(const char* cs_file_path)
{
	shaderID = LoadShader(cs_file_path, GL_COMPUTE_SHADER);
}

void µTec::ComputeShader::activate()
{
	glUseProgram(shaderID);
}

GLuint µTec::ComputeShader::LoadShader(const char* file_path, size_t type)
{
	GLuint subShaderID = glCreateShader(type);
	//GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string shaderCode;
	std::ifstream shaderStream(file_path, std::ios::in);
	if (shaderStream.is_open())
	{
		std::string Line = "";
		while (getline(shaderStream, Line))
			shaderCode += "\n" + Line;
		shaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Shader
	printf("Compiling shader : %s\n", file_path);
	char const * sourcePointer = shaderCode.c_str();
	glShaderSource(subShaderID, 1, &sourcePointer, NULL);
	glCompileShader(subShaderID);

	// Check Shader
	glGetShaderiv(subShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(subShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> shaderErrorMessage(InfoLogLength);
	if (shaderErrorMessage.size() > 0)
	{
		glGetShaderInfoLog(subShaderID, InfoLogLength, NULL, &shaderErrorMessage[0]);
		fprintf(stdout, "%s\n", &shaderErrorMessage[0]);
	}

	GLuint progHandle = glCreateProgram();
	int rvalue;
	glAttachShader(progHandle, subShaderID);

	glLinkProgram(progHandle);
	glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in linking compute shader program\n");
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog(progHandle, 10239, &length, log);
		fprintf(stderr, "Linker log:\n%s\n", log);

	}

	return progHandle;
}
