// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "RenderableFactory.h"

namespace µTec
{
	

	struct Bounds
	{
		Bounds() : sphere(), forward(), up(), right()
		{}
		glm::vec4 sphere;
		glm::vec4 forward;
		glm::vec4 up;
		glm::vec4 right;
	};

	struct WorldFieldUpdateParams
	{
		WorldFieldUpdateParams() : offset(), size()
		{}
		glm::ivec3 offset;
		glm::ivec3 size;

		bool operator== (WorldFieldUpdateParams& other)
		{
			return other.offset == this->offset && other.size == this->size;
		}
	};

	class Fields
	{
	public:
		Fields() : dimensions(), resolutions(), offsets(), positions(), rotations(), colors(), lightParams(), data(),
			fillColors(),textures(),scales(),cachedInverseTransforms(),cachedTransforms(),cachedRotations(),fieldBounds()
		{
		}

		void addFields(const size_t amount, const std::vector<Dimensions> dimension,
			const std::vector<float> resolution, std::vector<GLuint64> data, 
			const std::vector<std::string> tex, const std::vector<glm::vec4> col
			, const std::vector<glm::vec4> fillCol, const std::vector<glm::vec4> lightPar)
		{
			size_t totalSize = 0;
			glm::vec3 zero;
			glm::mat4 zeroMat;
			for (size_t i = 0; i < amount; ++i)
			{
				Dimensions currentDimension = dimension[i];
				offsets.push_back(totalSize);
				totalSize += currentDimension.length();
				dimensions.push_back(currentDimension);
				resolutions.push_back(resolution[i]);
				positions.push_back(zero);
				rotations.push_back(zero);
				scales.push_back(1);
				cachedRotations.push_back(zeroMat);
				cachedTransforms.push_back(zeroMat);
				cachedInverseTransforms.push_back(zeroMat);
				textures.push_back(tex[i]);
				colors.push_back(col[i]);
				fillColors.push_back(fillCol[i]);
				lightParams.push_back(lightPar[i]);
				fieldBounds.push_back(Bounds());
				oldParams.push_back(WorldFieldUpdateParams());
				needsUpdate.push_back(-1);
				this->data.push_back(data[i]);
			}

		}

		~Fields() 
		{
			clearFieldData();
		}

		void clearFieldData()
		{
		}

		size_t length()
		{
			return dimensions.size();
		}

		glm::mat4 getInverseTransform(size_t index)
		{
			return cachedInverseTransforms[index];
		}

		glm::mat4 getTransform(size_t index)
		{
			return cachedTransforms[index];
		}

		glm::mat4 getRotationTransform(size_t index)
		{
			return cachedRotations[index];
		}

		glm::vec3 getPosition(size_t index)
		{
			return positions[index];
		}

		glm::vec3 getRotation(size_t index)
		{
			return rotations[index];
		}

		float getScale(size_t index)
		{
			return scales[index];
		}

		float getResolution(size_t index)
		{
			return resolutions[index];
		}

		GLuint64 getData(size_t index)
		{
			return data[index];
		}

		Dimensions getDimensions(size_t index)
		{
			return dimensions[index];
		}

		size_t getGridLength(size_t index)
		{
			return getDimensions(index).length();
		}

		std::string getTextureName(size_t index)
		{
			return textures[index];
		}

		glm::vec4 getColor(size_t index)
		{
			return colors[index];
		}

		glm::vec4 getFillColor(size_t index)
		{
			return fillColors[index];
		}

		glm::vec4 getLightParams(size_t index)
		{
			return lightParams[index];
		}

		Bounds getFieldBounds(size_t index)
		{
			return fieldBounds[index];
		}

		void setPosition(size_t index, glm::vec3 pos)
		{
			positions[index] = pos;
			markAsDirty(index);
		}
	

		void setRotation(size_t index, glm::vec3 rot)
		{
			rotations[index] = rot;
			
			markAsDirty(index);
		}

		void setScale(size_t index, float sca)
		{
			scales[index] = sca;
			markAsDirty(index);
		}

		void recalculateTransforms(glm::vec4 time);

		bool objectNeedsUpdate(size_t index,WorldFieldUpdateParams& params, glm::ivec3& dispatchSize, glm::vec3& fieldOrigin,glm::vec3& fieldExtents);

		void CalculateWorldFieldUpdateParams(size_t index, glm::vec3& fieldOrigin, glm::vec3& fieldExtents, glm::ivec3& dispatchSize, WorldFieldUpdateParams &params);

		void markAsDirty(size_t index)
		{
			if (needsUpdate[index] == -1)
				needsUpdate[index] = UPDATE_FREQUENCY;
		}

		std::vector<GLuint64> data;

	private:

		const int UPDATE_FREQUENCY = 2;



		float getBoundingSphereRadius(float width, float height, float depth)
		{
			float diag = sqrt(width*width + height*height);
			float fulldiag = sqrt(diag*diag + depth*depth);
			return fulldiag*0.5;
		}

		std::vector<glm::mat4> cachedInverseTransforms;
		std::vector<glm::mat4> cachedTransforms;
		std::vector<glm::mat4> cachedRotations;
		std::vector<Dimensions> dimensions;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> rotations;
		std::vector<float> scales;
		std::vector<float> resolutions;
		std::vector<size_t> offsets;
		std::vector<std::string> textures;
		std::vector<glm::vec4> colors;
		std::vector<glm::vec4> fillColors;
		std::vector<glm::vec4> lightParams;
		std::vector<Bounds> fieldBounds;
		
		std::vector<WorldFieldUpdateParams> oldParams;
		std::vector<int> needsUpdate;
	};
}

