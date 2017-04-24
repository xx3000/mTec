// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once

#include "GlobalShader.h"
#include "RenderableFactory.h"
#include "Input.h"
#include "Event.h"
#include <stdio.h>
#include "Fields.h"
#include <queue>

namespace µTec
{

	

	struct RenderParams
	{
		glm::mat4 matVP;
		glm::vec4 camPos;
		glm::vec4 varParams;
		glm::vec3 lightPos;
		int objectLength;
	};

	struct FrustumParams
	{
		glm::vec4 FTL;
		glm::vec4 FTR;
		glm::vec4 FTC;
		glm::vec4 NTL;
		glm::vec4 NTR;
		glm::vec4 NTC;
	};

typedef Event<void( glm::mat4&,glm::mat4&, Renderable*)>  RenderEvent;

class Renderer
{
public:
	Renderer();
	~Renderer(void);

	void InitializeGlutCallbacks();

	void init(int* argc, char** argv,const char* windowName,unsigned windowWidth, unsigned windowHeight,bool fullscreen);

	void draw();

	void drawVelocity();

	void drawTAA();

	void SwapHistoryBuffers();

	void setMainCamera(Camera* cam)
	{
		mainCamera=cam;
	}

	RenderableFactory* getRenderableManager();
	void drawElementGL( VertexLayout* currentObject );
	void enableGLFlag(unsigned int flag);
	void disableGLFlag(unsigned int flag);
	void initShaders();

	void LoadBlurShader();

	void LoadReflectionShader();

	void LoadBlockMarchingShader();

	void LoadWorldIdShader();

	void LoadShadowFieldCullingShader();

	void LoadVelocityShader();

	void LoadReprojectionShader();

	void LoadShadowUpsamplingShader();

	void LoadUpsamplingShader();

	void LoadWorldSdfShader();

	void LoadCullingShader();

	void LoadAoShader();

	void LoadShadowShader();

	void LoadShadingShader();

	void BindFields(Fields* fields);
	void bindTex(const char* name, const char* path, GLuint* tex, bool isLinear = false);
	void LoadRMShader();

	void setQuad(Renderable* obj)
	{
		this->quad = obj;
		obj->calculateModelMatrix();
	}

	Fields* fields;
	std::vector<GLuint*> boundTextures;
	GLuint tex0;
	GLuint tex0_nrm;
	GLuint tex1;
	GLuint tex1_nrm;
	GLuint tex2;
	GLuint tex2_nrm;
	GLuint sdfBuffer;
	GLuint texBuffer;
	GLuint norMapBuffer;
	GLuint roughBuffer;
	GLuint metalBuffer;
	GLuint objectBuffer;
	GLuint transformBuffer;
	GLuint rotBuffer;
	GLuint paramBuffer;
	GLuint reproParamsBuffer;

	GLuint boundingSphereBuffer;
	GLuint frustumPointsBuffer;
	GLuint culledSdfBuffer;
	GLuint culledAoBuffer;
	GLuint worldSdfBuffer;
	GLuint worldSdfBuffer2;
	GLuint worldIdBuffer;

	GLuint posBuffer;
	GLuint* pastHistoryBuffer;
	GLuint* currentHistoryBuffer;
	GLuint utilBuffer1;
	GLuint utilBuffer2;
	GLuint velocityBuffer;
	GLuint normalBuffer;
	GLuint thicknessBuffer;
	GLuint idBuffer;
	GLuint reflectionBuffer;
	GLuint blurBuffer;

	GLuint shadowBuffer;
	GLuint filteredAoBuffer;
	GLuint filteredShadowBuffer;
	GLuint aoBuffer;
	GLuint sssBuffer;

	RayMarchingComputeShader* m_rmShader= nullptr;
	ShadowsComputeShader* m_shadowShader = nullptr;
	ShadingShader* m_shadingShader = nullptr;
	AoComputeShader* m_aoShader = nullptr;
	SssComputeShader* m_sssShader = nullptr;
	FieldCullingComputeShader* m_fcShader = nullptr;
	AoFieldCullingShader* m_aofcShader = nullptr;
	WorldSdfShader* m_worldShader = nullptr;
	UpsamplingShader* m_upsamplingShader = nullptr;
	UpsamplingShader* m_shadowupsamplingShader = nullptr;
	ReprojectionShader* m_reprojectionShader = nullptr;
	VelocityComputeShader* m_velocityShader = nullptr;
	ShadowFieldCullingShader* m_sfcShader = nullptr;
	WorldIdShader* m_worldIdShader = nullptr;
	BlockMarchingShader* m_blockMarchingShader = nullptr;
	ImplicitBlockMarchingShader* m_ImplBlockMarchingShader = nullptr;
	ReflectionComputeShader* m_reflectionShader = nullptr;
	BlurShader* m_blurShader = nullptr;

	float passTime;
	float debugTime;
	float timeSpeed;
	float timecount;
	float rm_shadows;
	float rm_ambientocclusion;
	float rm_translucency;
	float rm_debugsteps;
	bool animate;

	glm::vec4 shaderParams = glm::vec4(0, 0, 0, 0);

	std::queue<WorldFieldUpdateParams> updateList;

private:

	glm::mat4 historyV;
	glm::mat4 cachedVP;
	FrameBuffer shadingFB;
	FrameBuffer postaaFB;
	FrameBuffer marchFB;
	glm::vec3 cachedLightPos;

	Camera* mainCamera;

	RenderableFactory* m_factory;
	RenderParams params;
	FrustumParams frustum;
	Renderable* quad;
	Renderable* cube;

	const int ComputeDispatchX = 240; // 240 - 1920 / 8
	const int ComputeDispatchY = 135; // 135 - 1080 / 8
	bool TickTock = true;

	void drawRayMarching(Renderable* currentObject);

	void ComputeHits(glm::mat4 VP);

	void UpdateWorldDistanceField();

	void UpdateUniforms(glm::mat4 VP);

	void ComputeEffects();

	void RenderLighting(glm::mat4 VP, Renderable* currentObject);

	void DrawMesh(Renderable* mesh);

	void UpdateTransformBuffer();

	float GetPixelExtents(glm::mat4 VP, float z);

};

}