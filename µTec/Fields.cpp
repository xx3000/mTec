// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#include "Fields.h"
#include <iostream>
void µTec::Fields::recalculateTransforms(glm::vec4 time)
{
	glm::mat4 id, rotate, scale, center;
	glm::vec4 forward, up, right;
	for (size_t i = 0; i < length(); ++i)
	{
		Dimensions dim = getDimensions(i);
		glm::vec3 pos = positions[i];
		float scaledRes = getResolution(i)*getScale(i);
		scale = glm::scale(id, glm::vec3(dim.width, dim.height, dim.depth)*scaledRes);
		rotate = glm::toMat4(glm::quat(rotations[i]));
		cachedRotations[i] = glm::inverse(rotate);
		center = glm::translate(id, pos);
		cachedTransforms[i] = center*rotate*scale;
		cachedInverseTransforms[i] = glm::inverse(center*rotate);

		//cachedTransforms[i][3] = time;
		if (i ==4)
		cachedInverseTransforms[i][0][3] = time.y;

		
		float radius = getBoundingSphereRadius(dim.width, dim.height, dim.depth);
		fieldBounds[i].sphere = glm::vec4(pos.x, pos.y, pos.z, radius*scaledRes);
		right = glm::vec4(1, 0, 0, 0);
		up = glm::vec4(0, 1, 0, 0);
		forward = glm::vec4(0, 0, 1, 0);
		fieldBounds[i].forward = rotate*forward;
		fieldBounds[i].forward.w = 0.5f*scaledRes*(float)dim.depth;
		fieldBounds[i].up = rotate*up;
		fieldBounds[i].up.w = 0.5f*scaledRes*(float)dim.height;
		fieldBounds[i].right = rotate*right;
		fieldBounds[i].right.w = 0.5f*scaledRes*(float)dim.width;

	}
}

bool µTec::Fields::objectNeedsUpdate(size_t index, WorldFieldUpdateParams& params, glm::ivec3& dispatchSize, glm::vec3& fieldOrigin, glm::vec3& fieldExtents)
{
	int counter = needsUpdate[index];
	if (counter == -1)
		return false;


	if (counter == UPDATE_FREQUENCY)
	{
		WorldFieldUpdateParams oldParam;
		CalculateWorldFieldUpdateParams(index, fieldOrigin, fieldExtents, dispatchSize, oldParam);
		oldParams[index]=oldParam;
	}

	needsUpdate[index]--;

	if (counter == 1 && oldParams[index].size.length() != 0)
	{
		params = oldParams[index];
		return true;
	}

	if (counter != 0)
		return false;


	CalculateWorldFieldUpdateParams(index, fieldOrigin, fieldExtents, dispatchSize, params);

	if (oldParams[index] == params)
		return false;

	oldParams[index] = WorldFieldUpdateParams();

	return true;
}

void µTec::Fields::CalculateWorldFieldUpdateParams(size_t index, glm::vec3& fieldOrigin, glm::vec3& fieldExtents, glm::ivec3& dispatchSize, WorldFieldUpdateParams &params)
{
	Bounds bound = getFieldBounds(index);

	//glm::vec3 fwd = glm::vec3(bound.forward.x, bound.forward.y, bound.forward.z)*bound.sphere.w;
	//glm::vec3 rght = glm::vec3(bound.right.x, bound.right.y, bound.right.z)*bound.sphere.w;
	//glm::vec3 up = glm::vec3(bound.up.x, bound.up.y, bound.up.z)*bound.sphere.w;
	glm::vec3 fwd = glm::vec3(bound.forward.x, bound.forward.y, bound.forward.z)*bound.forward.w;
	glm::vec3 rght = glm::vec3(bound.right.x, bound.right.y, bound.right.z)*bound.right.w;
	glm::vec3 up = glm::vec3(bound.up.x, bound.up.y, bound.up.z)*bound.up.w;
	glm::vec3 pos = getPosition(index);
	glm::vec3 c1 = pos + fwd + up + rght;
	glm::vec3 c2 = pos + fwd + up - rght;
	glm::vec3 c3 = pos + fwd - up + rght;
	glm::vec3 c4 = pos + fwd - up - rght;
	glm::vec3 c5 = pos - fwd + up + rght;
	glm::vec3 c6 = pos - fwd + up - rght;
	glm::vec3 c7 = pos - fwd - up + rght;
	glm::vec3 c8 = pos - fwd - up - rght;
	glm::vec3 aabbMin =
		glm::vec3(
		glm::min(glm::min(glm::min(c1.x, c2.x), glm::min(c3.x, c4.x)), glm::min(glm::min(c5.x, c6.x), glm::min(c7.x, c8.x))),
		glm::min(glm::min(glm::min(c1.y, c2.y), glm::min(c3.y, c4.y)), glm::min(glm::min(c5.y, c6.y), glm::min(c7.y, c8.y))),
		glm::min(glm::min(glm::min(c1.z, c2.z), glm::min(c3.z, c4.z)), glm::min(glm::min(c5.z, c6.z), glm::min(c7.z, c8.z))));
	glm::vec3 aabbMax =
		glm::vec3(
		glm::max(glm::max(glm::max(c1.x, c2.x), glm::max(c3.x, c4.x)), glm::max(glm::max(c5.x, c6.x), glm::max(c7.x, c8.x))),
		glm::max(glm::max(glm::max(c1.y, c2.y), glm::max(c3.y, c4.y)), glm::max(glm::max(c5.y, c6.y), glm::max(c7.y, c8.y))),
		glm::max(glm::max(glm::max(c1.z, c2.z), glm::max(c3.z, c4.z)), glm::max(glm::max(c5.z, c6.z), glm::max(c7.z, c8.z))));

	glm::vec3 dim = glm::abs(aabbMax - aabbMin);


	glm::vec3 worldBoxOrigin = fieldOrigin - fieldExtents*0.5f;
	glm::vec3 boxSize = fieldExtents / (glm::vec3(dispatchSize));

	glm::vec3 adjustedPos = aabbMin - worldBoxOrigin;
	glm::ivec3 minBoxId = glm::max(glm::ivec3(0), glm::ivec3(adjustedPos / boxSize) - glm::ivec3(1));
	glm::ivec3 odispatchSize = glm::ivec3(dim / boxSize + glm::vec3(2));

	params.offset = minBoxId;
	params.size = odispatchSize;
}
