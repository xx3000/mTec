// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"

#include <vector>

namespace µTec
{

enum LightType {POINT_LIGHT=0,DIRECTIONAL_LIGHT,SPOT_LIGHT,AREA_LIGHT};

	class BaseShader
	{
	public:

		GLuint shaderID;


		BaseShader(const char * vertex_file_path,const char * fragment_file_path);
		BaseShader(const char * vertex_file_path,const char * geometry_file_path,const char * fragment_file_path);
		~BaseShader(void);

		void activate()
		{
			glUseProgram(shaderID);
		}

		GLuint linkShaders(std::vector<GLuint> subShaderID);
		GLuint LoadShader( const char * file_path, unsigned int type);
		const char* m_vertex_shader_path;
		const char* m_geometry_shader_path;
		const char* m_fragment_shader_path;

	};

	class ComputeShader
	{
	public:
		ComputeShader(const char* cs_file_path);

		void activate();

	protected:

		GLuint64 shaderID;

		GLuint LoadShader(const char* file_path, size_t type);


	};

}






