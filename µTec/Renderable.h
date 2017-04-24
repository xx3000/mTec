// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Vertex.h"
#include <map>

namespace µTec
{

	

struct VertexLayout
{
	VertexLayout(unsigned int indexCount, unsigned int verticesCount): vertexCount(verticesCount), indexCount(indexCount) 
	{
		vertices= new Vertex[vertexCount]();
		indices= new unsigned int[indexCount]();
	}
	~VertexLayout()
	{
		//TODO: Fix this allocation /deletion mess
		delete[] vertices;
		delete[] indices;
	}
	Vertex* vertices;
	unsigned int* indices;
	const unsigned int vertexCount;
	const unsigned int indexCount;
	unsigned int indexOffset;
	unsigned int vertexOffset;
};


struct Renderable
{
	Renderable() : position(glm::vec3(0,0,0))
		, rotation(glm::vec3(0,0,0)), scale(glm::vec3(1,1,1)) , m_isDirty(false)
	{
		
	};
	~Renderable()
	{
		
	}

	Renderable(const Renderable& other): texture(other.texture),textureObject(other.textureObject),name(other.name),position(other.position), rotation(other.rotation), scale(other.scale), defaultLayout(other.defaultLayout), m_isDirty(other.m_isDirty), normalMap(other.normalMap), normalTextureObject(other.normalTextureObject)
	{
		
	}

	DynamicImage* texture;
	DynamicImage* normalMap;
	GLuint textureObject;
	GLuint normalTextureObject;
	const char* name;
	VertexLayout* defaultLayout;


	glm::mat4 model;
	glm::mat4 cachedMVP;

	void setPosition(glm::vec3 pos)
	{
		position=pos;
		m_isDirty=true;
	}

	void setRotation(glm::vec3 rot)
	{
		rotation=rot;
		m_isDirty=true;
	}

	void setScale(glm::vec3 sca)
	{
		scale=sca;
		m_isDirty=true;
	}

	glm::vec3 getPosition()
	{
		return position;
	}

	glm::vec3* getPositionAddr()
	{
		return &position;
	}

	glm::vec3 getRotation()
	{
		return rotation;
	}

	glm::vec3 getScale()
	{
		return scale;
	}

	bool isDirty()
	{
		return m_isDirty;
	}
	
	void calculateModelMatrix()
	{
		glm::mat4 id,scaleM,rotate,center;
		scaleM=glm::scale(id,scale);
		rotate=glm::toMat4(glm::quat(rotation));
		center=glm::translate(id,position);
		model=center*rotate*scaleM;
		m_isDirty=false;
	}

private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	bool m_isDirty;
};


struct Waypoint 
{
	Renderable* gizmo;
	glm::vec3 position;
};

struct sort_byTextureId
{
	inline bool operator() (const Renderable* mesh1, const Renderable* mesh2)
	{
		return (mesh1->textureObject < mesh2->textureObject);
	}
};

}