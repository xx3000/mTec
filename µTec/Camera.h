// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once
#include <GL/freeglut.h>
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

namespace µTec
{


class Camera
{
public:
	Camera(unsigned int width, unsigned int height);
	~Camera(void);

	void switchProjectionMatrix(bool orthogonal);

	void setOrientation(glm::vec3 target,glm::vec3 up, glm::vec3 right)
	{
		m_target=target;
		m_up=up;
		m_right=right;
	}

	void setPosition(glm::vec3 pos)
	{
		m_pos=pos;
	}

	void addToPosition(glm::vec3 add)
	{
		m_pos+=add;
	}

	glm::vec3 getPosition()
	{
		return m_pos;
	}

	glm::vec3 getTarget()
	{
		return m_target;
	}

	glm::vec3 getUp()
	{
		return m_up;
	}

	float getFoV()
	{
		return m_FoV;
	}

	glm::vec3 getRight()
	{
		return m_right;
	}

	glm::mat4 Camera::getViewMatrix(){
		return m_ViewMatrix;
	}
	glm::mat4 Camera::getProjectionMatrix(){
		return m_ProjectionMatrix;
	}

	float getAspectRatio()
	{
		return (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	}


	// Copyright (c) <2015> <Playdead> L. Pedersen MIT Licence

	void jitterFrustum()
	{
		// update motion dir
		{
			glm::vec3 oldWorld = focalMotionPos;
			glm::vec3 newWorld = m_nearClippingPlane * m_target;

			glm::vec4 oldPointTmp = (m_ViewMatrix * glm::vec4(oldWorld, 1));
			glm::vec3 oldPoint = glm::vec3(oldPointTmp) / oldPointTmp.w;
			glm::vec4 newPointTmp = (m_ViewMatrix * glm::vec4(newWorld, 1));
			glm::vec3 newPoint = glm::vec3(newPointTmp) / newPointTmp.w;
			glm::vec3 newDelta = newPoint - oldPoint;
			newDelta.z = 0;

			float mag = newDelta.length();
			if (mag != 0.0f)
			{
				glm::vec3 dir = glm::normalize(newDelta);
				if (glm::length2(dir) != 0.0f)
				{
					focalMotionPos = newWorld;
					focalMotionDir = glm::mix(focalMotionDir, dir, 0.2f); //TODO slerp?
				}
			}
		}

		// update jitter
			{
				activeIndex += 1;
				activeIndex %= pattern.size();

				glm::vec2 sample = pattern[activeIndex];
				activeSample.z = activeSample.x;
				activeSample.w = activeSample.y;
				activeSample.x = sample.x;
				activeSample.y = sample.y;

				float aspect = static_cast<GLfloat>(WINDOW_WIDTH) / static_cast<GLfloat>(WINDOW_HEIGHT);
				float extentY = tan(glm::radians(m_FoV) * 0.5f);
				float extentX = extentY*aspect;
				float texelSizeX = extentX / (0.5f * static_cast<GLfloat>(WINDOW_WIDTH));
				float texelSizeY = extentY / (0.5f * static_cast<GLfloat>(WINDOW_HEIGHT));
				float oneJitterX = texelSizeX * sample.x;
				float oneJitterY = texelSizeY * sample.y;

				float cf = m_nearClippingPlane;
				float cn = m_farClippingPlane;
				float xm = (oneJitterX - extentX) * cn;
				float xp = (oneJitterX + extentX) * cn;
				float ym = (oneJitterY - extentY) * cn;
				float yp = (oneJitterY + extentY) * cn;

				glm::mat4 proj = glm::frustum(xm,xp,ym,yp,cn,cf);
				m_ProjectionMatrix = proj;
			}
	}

	void computeMatrices();

	float m_nearClippingPlane;
	float m_farClippingPlane;
	const unsigned int WINDOW_WIDTH;
	const unsigned int WINDOW_HEIGHT;
	bool renderGizmos;

	glm::vec4 activeSample;// xy = current sample, zw = previous sample
private:

	void createPattern()
	{
		//Uniform4_Helix
		//pattern.push_back(glm::vec2(-0.25f, -0.25f));
		//pattern.push_back(glm::vec2(0.25f, 0.25f));
		//pattern.push_back(glm::vec2(-0.25f, 0.25f));
		//pattern.push_back(glm::vec2(-0.25f, 0.25f));

		//Halton_2_3
		InitializeHalton_2_3(16);
	}

	// http://en.wikipedia.org/wiki/Halton_sequence
	float HaltonSeq(int prime, int index = 1/* NOT! zero-based */)
	{
		float r = 0.0f;
		float f = 1.0f;
		int i = index;
		while (i > 0)
		{
			f /= prime;
			r += f * (i % prime);
			i = (int)glm::floor(i / (float)prime);
		}
		return r;
	}

	void InitializeHalton_2_3(int seq)
	{
		for (int i = 0, n = seq; i != n; i++)
		{
			float u = HaltonSeq(2, i + 1) - 0.5f;
			float v = HaltonSeq(3, i + 1) - 0.5f;
			pattern.push_back(glm::vec2(u,v));
		}
	}

	glm::vec3 focalMotionPos;
	glm::vec3 focalMotionDir;

	
	int activeIndex = -1;

	glm::vec3 m_pos;
	glm::vec3 m_target;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ProjectionMatrix;
	std::vector<glm::vec2> pattern;

	float m_FoV;
};


}