// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once
#include "shader.h"
#include "glm/glm.hpp"

namespace µTec
{

	class ShadingShader :
		public BaseShader
	{
	public:
		GLuint positions;
		GLuint normals;
		GLuint shadows;
		GLuint thick;
		GLuint ids;
		GLuint ao;
		GLuint sss;
		GLuint reflections;
		GLuint gVPLocation;

		ShadingShader(void);
		~ShadingShader(void);
		void SetVP(const glm::mat4& WVP);
	private:
		void bindUniforms();
	};

	class ReprojectionShader :
		public BaseShader
	{
	public:

		GLuint frame;
		GLuint pastHistory;
		GLuint newHistory;
		GLuint gVPLocation;
		GLuint positions;
		GLuint velocity;

		ReprojectionShader(void);
		~ReprojectionShader(void);
		void SetVP(const glm::mat4& WVP);
	private:
		void bindUniforms();
	};

	class BlockMarchingShader :
		public BaseShader
	{
	public:

		GLuint gVPLocation;
		GLuint wLocation;
		GLuint index;
		GLuint field;
		GLuint invWorld;


		BlockMarchingShader(void);
		~BlockMarchingShader(void);
		void SetVP(const glm::mat4& VP);
		void SetWorld(const glm::mat4& W);
		void SetInverseWorld(const glm::mat4& W);
		void SetIndex(int id);
		void SetSDF(GLuint64 handle);
	private:
		void bindUniforms();
	};

	class ImplicitBlockMarchingShader :
		public BaseShader
	{
	public:

		GLuint gVPLocation;
		GLuint index;

		ImplicitBlockMarchingShader(void);
		~ImplicitBlockMarchingShader(void);
		void SetVP(const glm::mat4& VP);
		void SetIndex(int id);
	private:
		void bindUniforms();
	};

class RayMarchingComputeShader :
	public ComputeShader
{
public:
	GLuint Texture0;
	GLuint Texture1;
	GLuint Texture2;
	GLuint WorldSdf;
	GLuint WorldSdfIds;
	GLuint gVPLocation;
	GLuint time;
	GLuint effectToggle;
	GLuint objLength;
	GLuint posBuffer;
	GLuint normalBuffer;
	GLuint renderDimensions;
	GLuint thickBuffer;
	GLuint idBuffer;

	RayMarchingComputeShader() : ComputeShader("rayMarchingCs.shader")
	{
		activate();
		Texture0 = glGetUniformLocation(shaderID, "Texture0");
		Texture1 = glGetUniformLocation(shaderID, "Texture1");
		Texture2 = glGetUniformLocation(shaderID, "Texture2");
		WorldSdf = glGetUniformLocation(shaderID, "world");
		WorldSdfIds = glGetUniformLocation(shaderID, "worldIds");
		gVPLocation = glGetUniformLocation(shaderID, "matVP");
		time = glGetUniformLocation(shaderID, "time");
		effectToggle = glGetUniformLocation(shaderID, "sh_ao_tl_sd");	
		objLength = glGetUniformLocation(shaderID, "objectLength");
		posBuffer = glGetUniformLocation(shaderID, "posBuffer");
		normalBuffer = glGetUniformLocation(shaderID, "normalBuffer");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
		thickBuffer = glGetUniformLocation(shaderID, "ThickBuffer");
		idBuffer = glGetUniformLocation(shaderID, "IdBuffer");
	}
	void SetVP(const glm::mat4& WVP);
	void SetTime(const glm::vec4& t);
	void SetObjectLength(const int& len);
	void ToggleEffects(const glm::vec4& eff);
	void SetRenderDimensions(const glm::vec2 eff);
private:

};

class ReflectionComputeShader :
	public ComputeShader
{
public:
	GLuint positions;
	GLuint normals;
	GLuint WorldSdf;
	GLuint WorldSdfIds;
	GLuint refBuffer;
	GLuint renderDimensions;
	GLuint ids;

	ReflectionComputeShader() : ComputeShader("reflectionCs.shader")
	{
		activate();
		WorldSdf = glGetUniformLocation(shaderID, "world");
		WorldSdfIds = glGetUniformLocation(shaderID, "worldIds");
		refBuffer = glGetUniformLocation(shaderID, "refBuffer");
		positions = glGetUniformLocation(shaderID, "positions");
		normals = glGetUniformLocation(shaderID, "normals");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
		ids = glGetUniformLocation(shaderID, "idbuf");
	}
	void SetRenderDimensions(const glm::vec2 eff);
private:

};

class VelocityComputeShader :
	public ComputeShader
{
public:
	GLuint positions;
	GLuint renderDimensions;
	GLuint historyVP;
	GLuint velocityBuffer;

	VelocityComputeShader() : ComputeShader("velocityCs.shader")
	{
		activate();
		positions = glGetUniformLocation(shaderID, "positions");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
		historyVP = glGetUniformLocation(shaderID, "historyVP");
		velocityBuffer = glGetUniformLocation(shaderID, "velocityBuffer");
	}
	void SetRenderDimensions(const glm::vec2 eff);
	void SetHistoryVP(const glm::mat4& VP);
private:
};

class ShadowsComputeShader :
	public ComputeShader
{
public:
	GLuint positions;
	GLuint normals;
	GLuint shadowBuffer;
	GLuint WorldSdf;
	GLuint WorldIds;
	GLuint renderDimensions;

	ShadowsComputeShader() : ComputeShader("shadowsCs.shader")
	{
		activate();
		WorldSdf = glGetUniformLocation(shaderID, "world");
		positions = glGetUniformLocation(shaderID, "positions");
		normals = glGetUniformLocation(shaderID, "normals");
		shadowBuffer = glGetUniformLocation(shaderID, "shadowBuffer");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
		WorldIds = glGetUniformLocation(shaderID, "worldIds");
	}
	void SetRenderDimensions(const glm::vec2 eff);
private:
};

class AoComputeShader :
	public ComputeShader
{
public:
	GLuint positions;
	GLuint normals;
	GLuint gVPLocation;
	GLuint aoBuffer;
	GLuint WorldSdf;
	GLuint WorldIds;
	GLuint renderDimensions;

	AoComputeShader() : ComputeShader("aoCs.shader")
	{
		activate();
		WorldSdf = glGetUniformLocation(shaderID, "world");
		positions = glGetUniformLocation(shaderID, "positions");
		normals = glGetUniformLocation(shaderID, "normals");
		gVPLocation = glGetUniformLocation(shaderID, "matVP");
		aoBuffer = glGetUniformLocation(shaderID, "aoBuffer");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
		WorldIds = glGetUniformLocation(shaderID, "worldIds");
	}
	void SetRenderDimensions(const glm::vec2 eff);
	void SetVP(const glm::mat4& WVP);
private:
};

class SssComputeShader :
	public ComputeShader
{
public:
	GLuint positions;
	GLuint normals;
	GLuint gVPLocation;
	GLuint sssBuffer;
	GLuint renderDimensions;

	SssComputeShader() : ComputeShader("sssCs.shader")
	{
		activate();
		positions = glGetUniformLocation(shaderID, "positions");
		normals = glGetUniformLocation(shaderID, "normals");
		gVPLocation = glGetUniformLocation(shaderID, "matVP");
		sssBuffer = glGetUniformLocation(shaderID, "sssBuffer");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
	}
	void SetRenderDimensions(const glm::vec2 eff);
	void SetVP(const glm::mat4& WVP);
private:
};

class FieldCullingComputeShader :
	public ComputeShader
{
public:
	GLuint coneAngle;
	GLuint renderDimensions;
	FieldCullingComputeShader() : ComputeShader("fieldCompactorCs.shader")
	{
		activate();
		coneAngle = glGetUniformLocation(shaderID, "cone_cos_sin_tan");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
	}
	void SetRenderDimensions(const glm::vec2 eff);
	void SetConeAngles(const glm::vec3& angles);
private:
};

class AoFieldCullingShader :
	public ComputeShader
{
public:
	GLuint positions;
	GLuint renderDimensions;

	AoFieldCullingShader() : ComputeShader("aoFieldCullingCs.shader")
	{
		activate();
		positions = glGetUniformLocation(shaderID, "positions");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
	}
	void SetRenderDimensions(const glm::vec2 eff);
private:
};

class ShadowFieldCullingShader :
	public ComputeShader
{
public:
	GLuint positions;
	GLuint renderDimensions;

	ShadowFieldCullingShader() : ComputeShader("shadowsFieldCullingCs.shader")
	{
		activate();
		positions = glGetUniformLocation(shaderID, "positions");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
	}
	void SetRenderDimensions(const glm::vec2 eff);
private:
};

class WorldSdfShader :
	public ComputeShader
{
public:
	GLuint worldSdf;
	GLuint worldSdf2;
	GLuint offset;
	GLuint totalGroups;
	glm::ivec3 dispatchSize;
	WorldSdfShader() : ComputeShader("worldSdfCs.shader")
	{
		activate();
		worldSdf = glGetUniformLocation(shaderID, "worldSdf");
		worldSdf2 = glGetUniformLocation(shaderID, "worldSdf2");
		offset = glGetUniformLocation(shaderID, "offset");
		totalGroups = glGetUniformLocation(shaderID, "totalGroups");
	}
	void SetGroups(const glm::ivec3 size);
	void SetOffset(const glm::ivec3 size);
private:
};

class WorldIdShader :
	public ComputeShader
{
public:
	GLuint worldIds;
	GLuint processedIds;
	GLuint offset;
	GLuint totalGroups;
	glm::ivec3 dispatchSize;
	WorldIdShader() : ComputeShader("worldIdCs.shader")
	{
		activate();
		worldIds = glGetUniformLocation(shaderID, "worldIds");
		processedIds = glGetUniformLocation(shaderID, "processedIds");
		offset = glGetUniformLocation(shaderID, "offset");
		totalGroups = glGetUniformLocation(shaderID, "totalGroups");
	}
	void SetGroups(const glm::ivec3 size);
	void SetOffset(const glm::ivec3 size);
private:
};

class UpsamplingShader :
	public ComputeShader
{
public:
	GLuint positions;
	GLuint normals;
	GLuint intensities;
	GLuint intensities2;
	GLuint upsampledBuffer;
	GLuint upsampledBuffer2;
	GLuint renderDimensions;

	UpsamplingShader() : ComputeShader("upsamplerCs.shader")
	{
		activate();
		positions = glGetUniformLocation(shaderID, "positions");
		normals = glGetUniformLocation(shaderID, "normals");
		intensities = glGetUniformLocation(shaderID, "intensities");
		intensities2 = glGetUniformLocation(shaderID, "intensities2");
		upsampledBuffer = glGetUniformLocation(shaderID, "upsampledBuffer");
		upsampledBuffer2 = glGetUniformLocation(shaderID, "upsampledBuffer2");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
	}
	void SetRenderDimensions(const glm::vec2 eff);
private:
};

class BlurShader :
	public ComputeShader
{
public:
	GLuint colors;
	GLuint blurred;
	GLuint renderDimensions;

	BlurShader() : ComputeShader("blur.shader")
	{
		activate();
		colors = glGetUniformLocation(shaderID, "colors");
		blurred = glGetUniformLocation(shaderID, "blurredBuffer");
		renderDimensions = glGetUniformLocation(shaderID, "renderDimensions");
	}
	void SetRenderDimensions(const glm::vec2 eff);
private:
};

class NullShader :
	public BaseShader
{
public:
	GLuint gWVPLocation;


	NullShader(void);
	~NullShader(void);
		void SetWVP(const glm::mat4& WVP);
private:
	void bindUniforms();

};

class FXAAShader :
	public BaseShader
{
public:
		GLuint Texture0;
	FXAAShader(void);
	~FXAAShader(void);
	void SetWVP( const glm::mat4& WVP );
private:

	GLuint gWVPLocation;
	void bindUniforms();

};

}