// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"

using namespace µTec;

Camera::~Camera(void)
{
}

void Camera::computeMatrices()
{
	// Camera matrix
	m_ViewMatrix       = glm::lookAt(
		m_pos,           
		m_pos+m_target, 
		m_up                  
		);
}

µTec::Camera::Camera(unsigned int width, unsigned int height) : WINDOW_WIDTH(width), WINDOW_HEIGHT(height),
m_nearClippingPlane(1.0f), m_farClippingPlane(100.0f),
m_pos(glm::vec3(0, 1, -5)), m_target(glm::vec3(0, 0, -1)), m_up(glm::vec3(0, 1, 0)), m_right(glm::vec3(1, 0, 0)), m_FoV(45.0f),
focalMotionPos(), focalMotionDir(glm::vec3(1, 0, 0)), activeIndex (-1)
{
	m_ProjectionMatrix = glm::perspective(m_FoV, static_cast<GLfloat>(WINDOW_WIDTH) / static_cast<GLfloat>(WINDOW_HEIGHT), m_nearClippingPlane, m_farClippingPlane);
	createPattern();
}

void µTec::Camera::switchProjectionMatrix(bool orthogonal)
{
	if(orthogonal) m_ProjectionMatrix= glm::ortho(0.0f,(float)WINDOW_WIDTH,(float)WINDOW_HEIGHT,0.0f,m_nearClippingPlane,m_farClippingPlane);
	else m_ProjectionMatrix = glm::perspective(m_FoV, static_cast<GLfloat>(WINDOW_WIDTH)/ static_cast<GLfloat>(WINDOW_HEIGHT), m_nearClippingPlane,m_farClippingPlane);
}
