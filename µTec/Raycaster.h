#pragma once
#include "glm/glm.hpp"
// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)

namespace µTec
{


struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
	bool operator==(const Ray& other)
	{
		if(origin==other.origin &&
			direction == other.direction)return true;
		return false;
	}
};



}