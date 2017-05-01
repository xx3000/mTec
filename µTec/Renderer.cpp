// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#include "RenderingHelperFunctions.h"
#include "Renderer.h"
#include "Gametime.h"
using namespace µTec;




Renderer::Renderer()
{
	m_factory=new RenderableFactory();
}


Renderer::~Renderer(void)
{
	delete m_factory;
	delete m_rmShader;
	for (size_t i=0; i < boundTextures.size();++i)
	{
		delete boundTextures[i];
	}
}

LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
LARGE_INTEGER Frequency;
int frameCount = 0;
float avgMs = 0.0f;

void initCounter()
{
	QueryPerformanceFrequency(&Frequency);
}

void startCounter()
{
	QueryPerformanceCounter(&StartingTime);
}

void endCounter(bool output)
{
	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	float ms = ElapsedMicroseconds.QuadPart / 1000.0;
	if (output)
	{
		avgMs += ms;
		if (frameCount >= 1000)
		{
			frameCount = 0;
			std::cout << "avg:" << avgMs / 1000.0f << std::endl;
			avgMs = 0;
		}
		frameCount++;
	}
}

void Renderer::init( int* argc, char** argv,const char* windowName,unsigned windowWidth, unsigned windowHeight,bool fullscreen )
{
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(windowWidth,windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowName);
	if(fullscreen) glutFullScreen();

	//glutSetCursor(GLUT_CURSOR_NONE); 

	InitializeGlutCallbacks();

	// Must be done after glut is initialized!
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		assert(true);
	}
	
	SetVsync(0);

	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glFrontFace(GL_CCW);
	// Enable depth test
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);
	//glEnable(GL_MULTISAMPLE_ARB);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_CLAMP);

	passTime = false;
	timeSpeed = 1.0;
	timecount = 0;
	animate = false;
	rm_ambientocclusion = 1;
	rm_shadows = 1.0;
	rm_translucency = 1;
	rm_debugsteps = 0;

	debugTime = 3.14159265f*0.75f;

	m_factory->generateUniformBuffer(&params, 1, paramBuffer,true);

	m_factory->generateFramebuffer(shadingFB, windowWidth, windowHeight, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE);
	m_factory->generateFramebuffer(postaaFB, windowWidth, windowHeight, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE);
	m_factory->generateFramebuffer(marchFB, windowWidth, windowHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT);

	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, nullptr, &utilBuffer1, GL_RGBA16F, GL_RGBA,true);
	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, nullptr, &utilBuffer2, GL_RGBA16F, GL_RGBA,true);
	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, nullptr, &normalBuffer, GL_RG16F, GL_RG);
	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, nullptr, &thicknessBuffer, GL_R8, GL_RED);
	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, nullptr, &idBuffer, GL_R8I, GL_RED_INTEGER);
	
	SwapHistoryBuffers();

	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH)/2, glutGet(GLUT_WINDOW_HEIGHT)/2);

	GLint size;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &size);
	std::cout << "GL_MAX_UNIFORM_BLOCK_SIZE  is " << size << " " << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLint val;
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_FRONT_LEFT, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &val);
	std::cout << "color encoding  is " << val << " " << std::endl;

	initCounter();

	GLint mem;
	glGetIntegerv(0x9049,&mem);
	std::cout << "Avail GPU mem  is " << mem << " " << std::endl;

}


void Renderer::draw()
{
	//m_factory->CheckGlError("catchall");
	startCounter();
	
	mainCamera->computeMatrices();
	if (rm_debugsteps)
	{
		mainCamera->jitterFrustum();
	}
	cachedVP = mainCamera->getProjectionMatrix()*mainCamera->getViewMatrix();
	//glDisable(GL_FRAMEBUFFER_SRGB);
	// Render to our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, shadingFB.frameBuffer);
	glViewport(0, 0, mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT);
	drawRayMarching(quad);

	if (rm_debugsteps)
	{
		//drawVelocity();

		drawTAA();
	}
	else
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, shadingFB.frameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postaaFB.frameBuffer);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glDrawBuffer(GL_BACK);
		glBlitFramebuffer(0, 0, mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, 0, 0, mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, postaaFB.frameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glBlitFramebuffer(0, 0, mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, 0, 0, mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	glutSwapBuffers();
	
	endCounter(rm_shadows == 1.0);
}

void Renderer::drawElementGL( VertexLayout* currentObject )
{
	glDrawElementsBaseVertex(GL_TRIANGLES, currentObject->indexCount,
		GL_UNSIGNED_INT,(const GLvoid*) (sizeof(unsigned int)*currentObject->indexOffset) , currentObject->vertexOffset);
}

void empty()
{
}

void Renderer::InitializeGlutCallbacks()
{
	glutSpecialFunc(IInput::SpecialKeyboardCB);
	glutPassiveMotionFunc(IInput::PassiveMouseCB);
	glutMotionFunc(IInput::PassiveMouseCB);
	glutKeyboardFunc(IInput::KeyboardCB);
	glutMouseFunc(IInput::mouseCB);
	glutDisplayFunc(&empty);
}


RenderableFactory* Renderer::getRenderableManager()
{
	return m_factory;
}

void Renderer::initShaders()
{
	{
		int size = ComputeDispatchX * ComputeDispatchY * 128 * 8*4;
		std::vector<int8_t> culledSdfVal(size, -1);

		m_factory->generateAndFillBuffer(culledSdfVal, culledSdfBuffer);
	}

	LoadWorldSdfShader();

	LoadWorldIdShader();


	LoadRMShader();

	LoadShadingShader();

	LoadShadowShader();

	LoadAoShader();

	LoadCullingShader();

	LoadUpsamplingShader();

	LoadShadowUpsamplingShader();

	LoadReprojectionShader();

	//LoadVelocityShader();

	LoadShadowFieldCullingShader();

	LoadBlockMarchingShader();

	LoadReflectionShader();

	//LoadBlurShader();

}

struct ReprojectionParams
{
	glm::mat4 matVP;
	glm::mat4 historyVP;
	glm::vec4 renderDimensions;
	glm::vec4 jitter;
	glm::vec4 sinTime;
}
reproParams;

void µTec::Renderer::drawVelocity()
{
	m_velocityShader->activate();
	glm::mat4 historyVP = mainCamera->getProjectionMatrix()*historyV;
	m_velocityShader->SetHistoryVP(historyVP);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, posBuffer);

	glBindImageTexture(0, velocityBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
	
	glDispatchCompute(ComputeDispatchX, ComputeDispatchY, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void µTec::Renderer::drawTAA()
{
	//activate reprojection shader
	// update relevant uniforms
	m_reprojectionShader->activate();
	m_reprojectionShader->SetVP(cachedVP);
	reproParams.historyVP = mainCamera->getProjectionMatrix()*historyV;
	reproParams.matVP = glm::inverse(cachedVP);
	reproParams.jitter = mainCamera->activeSample;
	glm::vec3 campos = mainCamera->getPosition();
	reproParams.sinTime = glm::vec4(sin(GameTime::timeFromStartup / 8.0), campos.x, campos.y, campos.z);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, reproParamsBuffer);

	glBindBuffer(GL_UNIFORM_BUFFER, reproParamsBuffer);
	ReprojectionParams* ptr = reinterpret_cast<ReprojectionParams*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(ReprojectionParams), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT));
	*ptr = reproParams;
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	//bind history buffer to img slot 0
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postaaFB.frameBuffer);

	glBindImageTexture(0, *currentHistoryBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	//bind shadingfb image to sampler unit
	//bind old history buffer to sampler unit
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadingFB.renderTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, *pastHistoryBuffer);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, posBuffer);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, velocityBuffer);
	
	DrawMesh(quad);

	historyV = mainCamera->getViewMatrix();
	SwapHistoryBuffers();
}

void µTec::Renderer::SwapHistoryBuffers()
{
	if (TickTock)
	{
		currentHistoryBuffer = &utilBuffer2;
		pastHistoryBuffer = &utilBuffer1;
	}
	else
	{
		currentHistoryBuffer = &utilBuffer1;
		pastHistoryBuffer = &utilBuffer2;
	}
	TickTock = !TickTock;
}

void µTec::Renderer::enableGLFlag(unsigned int flag)
{
	glEnable(flag);
}

void µTec::Renderer::disableGLFlag( unsigned int flag )
{
	glDisable(flag);
}

void µTec::Renderer::LoadBlurShader()
{
	delete m_blurShader;
	m_blurShader = new BlurShader();
	m_factory->CheckGlError("load shader 5");
	m_blurShader->SetRenderDimensions(glm::vec2(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT));
	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, nullptr, &blurBuffer, GL_R11F_G11F_B10F, GL_RGB);
	glUniform1i(m_blurShader->colors, 0);
}

void µTec::Renderer::LoadReflectionShader()
{
	delete m_reflectionShader;
	m_reflectionShader = new ReflectionComputeShader();
	m_factory->CheckGlError("load shader 5");
	m_reflectionShader->SetRenderDimensions(glm::vec2(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT));
	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, nullptr, &reflectionBuffer, GL_R11F_G11F_B10F, GL_RGB);

	glUniform1i(m_reflectionShader->positions, 0);
	glUniform1i(m_reflectionShader->normals, 1);
	glUniform1i(m_reflectionShader->WorldSdf, 3);
	glUniform1i(m_reflectionShader->WorldSdfIds, 4);
	glUniform1i(m_reflectionShader->ids, 5);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, sdfBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, texBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, objectBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, paramBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, culledSdfBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, norMapBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, roughBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, metalBuffer);

}

void µTec::Renderer::LoadBlockMarchingShader()
{
	delete m_blockMarchingShader;
	m_blockMarchingShader = new BlockMarchingShader();
	m_factory->CheckGlError("load shader 5");


	delete m_ImplBlockMarchingShader;
	m_ImplBlockMarchingShader = new ImplicitBlockMarchingShader();

}

void µTec::Renderer::LoadWorldIdShader()
{
	delete m_worldIdShader;
	m_worldIdShader = new WorldIdShader();
	m_worldIdShader->activate();

	glUniform1i(m_worldIdShader->worldIds, 0);
	glUniform1i(m_worldIdShader->processedIds, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, paramBuffer);

	m_worldIdShader->SetGroups(glm::ivec3(128, 64, 128));
	m_factory->CheckGlError("load shader 5");
}

void µTec::Renderer::LoadShadowFieldCullingShader()
{
	delete m_sfcShader;

	m_sfcShader = new ShadowFieldCullingShader();
	m_sfcShader->activate();
	glUniform1i(m_sfcShader->positions, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 10, boundingSphereBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, culledSdfBuffer);
	m_sfcShader->SetRenderDimensions(glm::vec2(glm::vec2(mainCamera->WINDOW_WIDTH*0.25, mainCamera->WINDOW_HEIGHT*0.25)));

}

void µTec::Renderer::LoadVelocityShader()
{
	delete m_velocityShader;
	m_velocityShader = new VelocityComputeShader();
	m_velocityShader->activate();
	glUniform1i(m_velocityShader->positions, 0);
	glUniform1i(m_velocityShader->velocityBuffer, 0);
	m_velocityShader->SetRenderDimensions(glm::vec2(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT));

	m_factory->CheckGlError("load shader 4");
	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, nullptr, &velocityBuffer, GL_RG32F, GL_RG);

	m_factory->CheckGlError("load shader 5");

}

void µTec::Renderer::LoadReprojectionShader()
{
	delete m_reprojectionShader;

	m_reprojectionShader = new ReprojectionShader();
	m_reprojectionShader->activate();
	glUniform1i(m_reprojectionShader->frame, 0);
	glUniform1i(m_reprojectionShader->pastHistory, 1);
	glUniform1i(m_reprojectionShader->positions, 2);
	glUniform1i(m_reprojectionShader->velocity, 3);

	glUniform1i(m_reprojectionShader->newHistory, 0);

	reproParams.renderDimensions = glm::vec4(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, 1.0 / (float)mainCamera->WINDOW_WIDTH, 1.0 / (float)mainCamera->WINDOW_HEIGHT);
	m_factory->generateUniformBuffer(&reproParams, 1, reproParamsBuffer,true);

	m_factory->CheckGlError("load shader 5");
}

void µTec::Renderer::LoadShadowUpsamplingShader()
{
	delete m_shadowupsamplingShader;
	m_shadowupsamplingShader = new UpsamplingShader();
	m_shadowupsamplingShader->activate();

	glUniform1i(m_shadowupsamplingShader->positions, 0);
	glUniform1i(m_shadowupsamplingShader->normals, 1);
	glUniform1i(m_shadowupsamplingShader->intensities, 2);
	glUniform1i(m_shadowupsamplingShader->upsampledBuffer, 0);
	m_shadowupsamplingShader->SetRenderDimensions(glm::vec2(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT));

	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, nullptr, &filteredShadowBuffer, GL_R8, GL_RED, true);

	glBindImageTexture(0, filteredShadowBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);

	m_factory->CheckGlError("load shader 5");

}

void µTec::Renderer::LoadUpsamplingShader()
{
	delete m_upsamplingShader;

	m_upsamplingShader = new UpsamplingShader();
	m_upsamplingShader->activate();

	glUniform1i(m_upsamplingShader->positions, 0);
	glUniform1i(m_upsamplingShader->normals, 1);
	glUniform1i(m_upsamplingShader->intensities, 2);
	glUniform1i(m_upsamplingShader->upsampledBuffer, 0);
	m_upsamplingShader->SetRenderDimensions(glm::vec2(mainCamera->WINDOW_WIDTH*0.5, mainCamera->WINDOW_HEIGHT*0.5));

	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH*0.5, mainCamera->WINDOW_HEIGHT*0.5, nullptr, &filteredAoBuffer, GL_R8, GL_RED, true);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, filteredAoBuffer);
	glBindImageTexture(0, filteredAoBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);

	m_factory->CheckGlError("load shader 5");
}

void µTec::Renderer::LoadWorldSdfShader()
{
	delete m_worldShader;
	
	m_worldShader = new WorldSdfShader();
	m_worldShader->activate();

	int uniformDimension = 512;
	int size = uniformDimension * uniformDimension*0.5 * uniformDimension;
	//std::vector<__int16> test(size,0);
	float zero = 0.0;

	m_factory->bind3DBufferTexture(uniformDimension, uniformDimension*0.5, uniformDimension,&zero, &worldSdfBuffer, GL_R16F, GL_RED,GL_FLOAT);

	m_factory->bind3DBufferTexture(uniformDimension, uniformDimension*0.5, uniformDimension, &zero, &worldSdfBuffer2, GL_R8I, GL_RED_INTEGER, GL_BYTE, false);

	m_factory->bind3DBufferTexture(uniformDimension, uniformDimension*0.5, uniformDimension, &zero, &worldIdBuffer, GL_R8I, GL_RED_INTEGER, GL_BYTE, false);


	//glUniformHandleui64ARB(m_worldShader->worldSdf, worldSdfTex);
	m_factory->CheckGlError("load shader 4");
	glUniform1i(m_worldShader->worldSdf, 0);
	glUniform1i(m_worldShader->worldSdf2, 1);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, sdfBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, objectBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, paramBuffer);

	m_worldShader->SetGroups(glm::ivec3(127, 63, 127));

	WorldFieldUpdateParams params;
	params.offset = glm::ivec3(0);
	params.size = glm::ivec3(m_worldShader->dispatchSize);
	updateList.push(params);

	m_factory->CheckGlError("load shader 5");
}

void µTec::Renderer::LoadCullingShader()
{
	delete m_fcShader;

	m_fcShader = new FieldCullingComputeShader();
	m_fcShader->activate();
	m_fcShader->SetRenderDimensions(glm::vec2(240, 135));
	// compute frustum cone angle
	float halfHeight = mainCamera->WINDOW_HEIGHT*0.5;
	float halfTan = tan(glm::radians(mainCamera->getFoV()) * 0.5f);
	float toplen = halfHeight / halfTan;
	float cornerlen = halfHeight*sqrt(2);
	float cornerAngle = atan(cornerlen / toplen);
	float unitAngle = cornerAngle / ComputeDispatchY;
	float anglecos = cos(unitAngle);

	m_factory->generateUniformBuffer(&frustum, 1, frustumPointsBuffer,true);

	glm::vec3 angles = glm::vec3(anglecos*anglecos,1/sin(unitAngle), 1.0 / tan(unitAngle));
	m_fcShader->SetConeAngles(angles);

	glBindBufferBase(GL_UNIFORM_BUFFER, 10, boundingSphereBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, culledSdfBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, culledAoBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 14, frustumPointsBuffer);
}

void µTec::Renderer::LoadAoShader()
{
	delete m_aoShader;

	m_aoShader = new AoComputeShader();
	m_aoShader->activate();
	m_aoShader->SetRenderDimensions(glm::vec2(1920 * 0.25, 1080 * 0.25));
	glUniform1i(m_aoShader->positions, 0);
	glUniform1i(m_aoShader->normals, 1);
	glUniform1i(m_aoShader->WorldSdf, 3);
	glUniform1i(m_aoShader->WorldIds, 4);
	glUniform1i(m_aoShader->aoBuffer, 0);
	

	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH*0.25, mainCamera->WINDOW_HEIGHT*0.25, nullptr, &aoBuffer, GL_R8, GL_RED);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, aoBuffer);
	glBindImageTexture(0, aoBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, sdfBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, objectBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, paramBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, culledAoBuffer);

	m_factory->CheckGlError("load shader 5");
}

void µTec::Renderer::LoadShadowShader()
{
	delete m_shadowShader;
	m_shadowShader = new ShadowsComputeShader();
	m_shadowShader->activate();
	m_shadowShader->SetRenderDimensions(glm::vec2(1920 * 0.5, 1080 * 0.5));
	glUniform1i(m_shadowShader->positions, 0);
	glUniform1i(m_shadowShader->normals, 1);
	glUniform1i(m_shadowShader->WorldSdf, 3);
	glUniform1i(m_shadowShader->WorldIds, 4);
	glUniform1i(m_shadowShader->shadowBuffer, 0);

	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH*0.5, mainCamera->WINDOW_HEIGHT*0.5, nullptr, &shadowBuffer, GL_R8, GL_RED);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer);
	glBindImageTexture(0, shadowBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, sdfBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, objectBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, paramBuffer);

	m_factory->CheckGlError("load shader 5");
}

void µTec::Renderer::LoadShadingShader()
{
	delete m_shadingShader;
	m_shadingShader = new ShadingShader();
	m_shadingShader->activate();
	glUniform1i(m_shadingShader->positions, 0);
	glUniform1i(m_shadingShader->normals, 1);
	glUniform1i(m_shadingShader->shadows, 2);
	glUniform1i(m_shadingShader->ao, 3);
	glUniform1i(m_shadingShader->thick, 4);
	glUniform1i(m_shadingShader->reflections, 5);
	glUniform1i(m_shadingShader->ids, 11);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, texBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, objectBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, paramBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, norMapBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, roughBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, metalBuffer);

}


void µTec::Renderer::BindFields(Fields* fields)
{
	struct object{
		glm::vec4 resolution;
		glm::vec4 color;
		glm::vec4 fillColor;
		glm::vec4 lightParams;
		int fieldId;
		float texSize;
		int transId;
		int texId;
	} obj;

	this->fields = fields;

	m_factory->LoadTextures("textures");
	{
		boundTextures.clear();
		boundTextures.reserve(fields->length());
		GLuint64 tmpHandle;
		std::vector<GLuint64> texHandles;
		std::vector<GLuint64> norHandles;
		std::vector<GLuint64> roughHandles;
		std::vector<GLuint64> metalHandles;
		std::vector<object> objects;

		//setup world sdf

		m_factory->CheckGlError("catchall");


		//handles.push_back(worldSdfBuffer);
		m_factory->loadAndGetTextureHandle(fields->getTextureName(0).c_str(), "textures", &tmpHandle);
		texHandles.push_back(tmpHandle);
		m_factory->loadAndGetTextureHandle((fields->getTextureName(0) + "_nrm").c_str(), "textures", &tmpHandle, true);
		norHandles.push_back(tmpHandle);
		//m_factory->loadAndGetTextureHandle((fields->getTextureName(0) + "_rough").c_str(), "textures", &tmpHandle, true);
		roughHandles.push_back(tmpHandle);
		//m_factory->loadAndGetTextureHandle((fields->getTextureName(0) + "_metal").c_str(), "textures", &tmpHandle, true);
		metalHandles.push_back(tmpHandle);

		fields->setPosition(0, glm::vec3(0, 20, 0));
		float invres = 1.0f / 86.0f;
		glm::vec4 res = glm::vec4(invres, 1.0f / 43.0f, invres, 0.0);
		res.w = fields->getScale(0);
		obj.resolution = res;
		obj.color = fields->getColor(0);
		obj.color.w = 1 / fields->getScale(0);
		obj.fillColor = fields->getFillColor(0);
		obj.lightParams = fields->getLightParams(0);
		obj.fieldId = 0;
		obj.texSize = 0.5;
		obj.transId = 0;
		obj.texId = 1;
		objects.push_back(obj);
		

		bool instances = false;

		int len = instances ? 4 : fields->length();

		for (size_t i = 1; i < len; ++i)
		{
			Dimensions dim = fields->getDimensions(i);
			m_factory->loadAndGetTextureHandle(fields->getTextureName(i).c_str(), "textures", &tmpHandle);
			texHandles.push_back(tmpHandle);
			m_factory->loadAndGetTextureHandle((fields->getTextureName(i) + "_nrm").c_str(), "textures", &tmpHandle, true);
			norHandles.push_back(tmpHandle);
			m_factory->loadAndGetTextureHandle((fields->getTextureName(i) + "_rough").c_str(), "textures", &tmpHandle, true);
			roughHandles.push_back(tmpHandle);
			m_factory->loadAndGetTextureHandle((fields->getTextureName(i) + "_metal").c_str(), "textures", &tmpHandle, true);
			metalHandles.push_back(tmpHandle);

			glm::vec3 pos = fields->getPosition(i);


			glm::vec4 res = glm::vec4(dim.width, dim.height, dim.depth, 0.0);
			res *= fields->getResolution(i);
			res = 1.0f / res;
			res.w = fields->getScale(i);
			obj.resolution = res;
			obj.color = fields->getColor(i);
			obj.color.w = 1 / fields->getScale(i);
			obj.fillColor = fields->getFillColor(i);
			obj.lightParams = fields->getLightParams(i);
			obj.fieldId = i;
			obj.texSize = 0.5;
			obj.transId = i;
			obj.texId = i;
			objects.push_back(obj);
		}
		if (instances)
		{
			for (size_t i = 4; i < fields->length(); ++i)
			{
				Dimensions dim = fields->getDimensions(i);
				glm::vec3 pos = fields->getPosition(i);

				//m_factory->loadAndGetTextureHandle(fields->getTextureName(3).c_str(), "textures", &tmpHandle);
				texHandles.push_back(tmpHandle);
				//m_factory->loadAndGetTextureHandle((fields->getTextureName(3) + "_nrm").c_str(), "textures", &tmpHandle, true);
				norHandles.push_back(tmpHandle);
				roughHandles.push_back(tmpHandle);
				metalHandles.push_back(tmpHandle);

				glm::vec4 res = glm::vec4(dim.width, dim.height, dim.depth, 0.0);
				res *= fields->getResolution(i);
				res = 1.0f / res;
				res.w = fields->getScale(i);
				obj.resolution = res;
				obj.color = fields->getColor(i);
				obj.color.w = 1 / fields->getScale(i);
				obj.fillColor = fields->getFillColor(i);
				obj.lightParams = fields->getLightParams(i);
				obj.fieldId = i;
				obj.texSize = 0.5;
				obj.transId = i;
				obj.texId = 3;
				objects.push_back(obj);
			}
		}
		/**/

		m_factory->generateAndFillBuffer(texHandles, texBuffer);
		m_factory->generateAndFillBuffer(norHandles, norMapBuffer);
		m_factory->generateAndFillBuffer(roughHandles, roughBuffer);
		m_factory->generateAndFillBuffer(metalHandles, metalBuffer);
		m_factory->generateUniformBuffer(&objects[0], fields->length(), objectBuffer, true);
	}

	params.objectLength = fields->length();

	m_factory->generateUniformBuffer<glm::mat4>(&fields->getInverseTransform(0), fields->length(), transformBuffer,true);
	m_factory->generateUniformBuffer<glm::mat4>(&fields->getRotationTransform(0), fields->length(), rotBuffer, true);
	m_factory->generateUniformBuffer<Bounds>(&fields->getFieldBounds(0), fields->length(), boundingSphereBuffer, true);
	m_factory->generateAndFillBuffer(fields->data, sdfBuffer);
	
	m_factory->CheckGlError("Bind fields end");

	{
		m_factory->addStaticObjectsFromDirectory("meshes", "textures");

		Renderable* quad = m_factory->getStaticPrototypeByName("quad");
		cube = m_factory->getStaticPrototypeByName("cube");
		if (quad != nullptr)
		{
			setQuad(quad);
		}
		m_factory->deleteImages();
	}
	
	

}

void µTec::Renderer::LoadRMShader()
{
	delete m_rmShader;
	m_rmShader = new RayMarchingComputeShader();
	m_rmShader->activate();
	m_rmShader->SetRenderDimensions(glm::vec2(1920, 1080));
	//glUniformHandleui64ARB(m_rmShader->Texture0, worldSdfBuffer);
	glUniform1i(m_rmShader->WorldSdf, 3);
	glUniform1i(m_rmShader->WorldSdfIds, 4);

	glUniform1i(m_rmShader->posBuffer, 0);
	glUniform1i(m_rmShader->normalBuffer, 1);
	glUniform1i(m_rmShader->thickBuffer, 2);
	glUniform1i(m_rmShader->idBuffer, 3);

	m_factory->CheckGlError("load shader 2");
	
	m_factory->bindBufferTexture(mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT, nullptr, &posBuffer, GL_R32F, GL_RED,true);

	//glBindFramebuffer(GL_FRAMEBUFFER, postprocessFB.frameBuffer);
	m_factory->CheckGlError("load shader 4a");
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outTex, 0);
	m_factory->CheckGlError("load shader 4");
	//m_factory->loadAndBindTexture("lowland", "textures", &tex3);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, sdfBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, texBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, objectBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, paramBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, culledSdfBuffer);

	m_factory->CheckGlError("load shader 5");
	float time = (timecount / 20.0f) + 0.5f;
	//cachedLightPos = glm::normalize(glm::vec3(cos(time), sin(time), 0.5));
}

float rot = 0;

glm::vec4 time;
void µTec::Renderer::ComputeHits(glm::mat4 VP)
{
	if (passTime)
	{
		//debugTime = (sin(time*3.0) + 1.0)*0.5;
		debugTime += (float)GameTime::deltaTime*timeSpeed*0.05f;
		fields->markAsDirty(4);
	}
	if (animate)
	{
		rot += (float)GameTime::deltaTime;
		fields->setRotation(4, glm::vec3(0, rot*0.5, 0));
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, 6, transformBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 7, rotBuffer);

	

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, worldSdfBuffer);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_3D, worldIdBuffer);

	/*
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBindFramebuffer(GL_FRAMEBUFFER, marchFB.frameBuffer);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadingFB.frameBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	m_blockMarchingShader->activate();
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, sdfBuffer);
	m_blockMarchingShader->SetVP(cachedVP);
	//glViewport(0, 0, ComputeDispatchX, ComputeDispatchY);
	for (size_t i = 1; i < fields->length(); ++i)
	{
		m_blockMarchingShader->SetInverseWorld(fields->getInverseTransform(i));
		m_blockMarchingShader->SetSDF(fields->getData(i));
		m_blockMarchingShader->SetWorld(fields->getTransform(i));
		m_blockMarchingShader->SetIndex(i);
		DrawMesh(cube);
		//glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	
	m_ImplBlockMarchingShader->activate();
	m_ImplBlockMarchingShader->SetVP(VP);
	m_ImplBlockMarchingShader->SetIndex(0);
	DrawMesh(quad);

	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, shadingFB.frameBuffer);
	*/
	//glViewport(0, 0, mainCamera->WINDOW_WIDTH, mainCamera->WINDOW_HEIGHT);
	
	
	m_rmShader->activate();

	glBindImageTexture(0, posBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindImageTexture(1, normalBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	glBindImageTexture(2, thicknessBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
	glBindImageTexture(3, idBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8I);
	

	
	time.x = debugTime;
	time.y = (sin(time.x * 2.0f) + 1.0f)*0.5f;
	m_rmShader->SetTime(time);

	glDispatchCompute(ComputeDispatchX, ComputeDispatchY, 1);
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
}


void µTec::Renderer::UpdateWorldDistanceField()
{
	if (updateList.size() > 0)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 6, transformBuffer);
		glBindBufferBase(GL_UNIFORM_BUFFER, 7, rotBuffer);

		WorldFieldUpdateParams current = updateList.front();
		updateList.pop();
		m_worldShader->activate();
		m_worldShader->SetOffset(current.offset);
		glBindImageTexture(0, worldSdfBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
		glBindImageTexture(1, worldSdfBuffer2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8I);

		glDispatchCompute(current.size.x, current.size.y, current.size.z);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		m_worldIdShader->activate();
		m_worldIdShader->SetOffset(current.offset);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, worldSdfBuffer2);
		glBindImageTexture(0, worldIdBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8I);
		current.size += glm::ivec3(1);
		glDispatchCompute(current.size.x, current.size.y, current.size.z);
	}
}

void µTec::Renderer::UpdateUniforms(glm::mat4 VP)
{
	//cachedLightPos = glm::vec3(15.0*sin(GameTime::timeFromStartup*0.25), 10.0, 15.0*cos(GameTime::timeFromStartup*0.25));
	cachedLightPos = glm::vec3(0.0, 10.0, 3.0);

	float pixelExtents = GetPixelExtents(VP, 0.0);
	float pixelExtents2 = GetPixelExtents(VP, 1.0);
	glm::vec3 campos = mainCamera->getPosition();
	params.lightPos = cachedLightPos;
	params.camPos = glm::vec4(campos.x, campos.y, campos.z, pixelExtents);
	params.matVP = VP;
	params.varParams = shaderParams;

	glBindBuffer(GL_UNIFORM_BUFFER, paramBuffer);
	RenderParams* ptr = reinterpret_cast<RenderParams*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(RenderParams), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT));
	*ptr = params;
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	float halfTan = tan(glm::radians(mainCamera->getFoV()) * 0.5f);
	float Hnear = 2 * halfTan *mainCamera->m_nearClippingPlane;
	float Wnear = Hnear*mainCamera->getAspectRatio();

	float Hfar = 2 * halfTan *mainCamera->m_farClippingPlane;
	float Wfar = Hfar*mainCamera->getAspectRatio();
	glm::vec3 farC = mainCamera->getPosition() + mainCamera->getTarget()*mainCamera->m_farClippingPlane;
	glm::vec3 frustumFTL = farC + (mainCamera->getUp()*Hfar*0.5f) - (mainCamera->getRight() * Wfar*0.5f);
	glm::vec3 frustumFTR = frustumFTL + mainCamera->getRight() * Wfar;
	glm::vec3 frustumFBL = frustumFTL - mainCamera->getUp() * Hfar;

	glm::vec3 nearC = mainCamera->getPosition() + mainCamera->getTarget()*mainCamera->m_nearClippingPlane;
	glm::vec3 frustumNTL = nearC + (mainCamera->getUp()*Hnear*0.5f) - (mainCamera->getRight() * Wnear*0.5f);
	glm::vec3 frustumNTR = frustumNTL + mainCamera->getRight() * Wnear;
	glm::vec3 frustumNBL = frustumNTL - mainCamera->getUp() * Hnear;

	glm::vec3 target = mainCamera->getTarget();
	frustum.FTL = glm::vec4(frustumFTL, target.x);
	frustum.FTR = glm::vec4(frustumFTR - frustumFTL, target.y);
	frustum.FTC = glm::vec4(frustumFBL - frustumFTL, target.z);
	frustum.NTL = glm::vec4(frustumNTL, 0);
	frustum.NTR = glm::vec4(frustumNTR - frustumNTL, 0);
	frustum.NTC = glm::vec4(frustumNBL - frustumNTL, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, frustumPointsBuffer);
	FrustumParams* fptr = reinterpret_cast<FrustumParams*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(FrustumParams), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT));
	*fptr = frustum;
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

double effectsTime = 0;
void µTec::Renderer::ComputeEffects()
{
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, posBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_3D, worldSdfBuffer);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_3D, worldIdBuffer);
	
	m_reflectionShader->activate();
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, idBuffer);
	glBindImageTexture(0, reflectionBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, culledSdfBuffer);
	glDispatchCompute(ComputeDispatchX, ComputeDispatchY, 1);



	m_sfcShader->activate();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, culledSdfBuffer);
	glDispatchCompute(60, 34, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//m_aofcShader->activate();
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, culledAoBuffer);
	//glDispatchCompute(60, 34, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	
	m_shadowShader->activate();
	glBindImageTexture(0, shadowBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
	glDispatchCompute(120, 68, 1);
	
	
	m_aoShader->activate();
	glBindImageTexture(0, aoBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
	glDispatchCompute(60, 34, 1);
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	
	m_upsamplingShader->activate();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, aoBuffer);
	glBindImageTexture(0, filteredAoBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
	glDispatchCompute(120, 68, 1);
	
	
	m_shadowupsamplingShader->activate();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer);
	glBindImageTexture(0, filteredShadowBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
	glDispatchCompute(ComputeDispatchX, ComputeDispatchY, 1);

	//m_blurShader->activate();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, reflectionBuffer);
	//glBindImageTexture(0, blurBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
	//glDispatchCompute(ComputeDispatchX, ComputeDispatchY, 1);
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

}

void µTec::Renderer::RenderLighting(glm::mat4 VP, Renderable* currentObject)
{
	m_shadingShader->activate();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, posBuffer);
	//glBindTexture(GL_TEXTURE_2D, marchFB.renderTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalBuffer);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, filteredShadowBuffer);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, filteredAoBuffer);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, thicknessBuffer);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, reflectionBuffer);
	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, idBuffer);

	m_shadingShader->SetVP(VP);
	//m_defaultShader->SetLightPosition(cachedLightPos);
	//m_defaultShader->SetCameraPosition(glm::vec4(campos.x, campos.y, campos.z, pixelExtents));

	glBindBufferBase(GL_UNIFORM_BUFFER, 6, transformBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 7, rotBuffer);

	DrawMesh(quad);
}

void µTec::Renderer::DrawMesh(Renderable* mesh)
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, m_factory->defaultBuffers.staticVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_factory->defaultBuffers.staticIndexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(glm::vec3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

	drawElementGL(mesh->defaultLayout);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void µTec::Renderer::UpdateTransformBuffer()
{
	//fields->setPosition(0, mainCamera->getPosition()*-1.0f);


	fields->recalculateTransforms(glm::vec4(time.x,time.y,0,0));

	int length = fields->length();
	glBindBuffer(GL_UNIFORM_BUFFER, transformBuffer);
	GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT; // the invalidate makes a big difference when re-writing
	glm::mat4* ptr = reinterpret_cast<glm::mat4*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, length * sizeof(glm::mat4), bufMask));
	for (size_t i = 0; i < length; i++)
	{
		ptr[i] = fields->getInverseTransform(i);
	}
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	glBindBuffer(GL_UNIFORM_BUFFER, rotBuffer);
	ptr = reinterpret_cast<glm::mat4*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, length * sizeof(glm::mat4), bufMask));
	for (size_t i = 0; i < length; i++)
	{
		ptr[i] = fields->getRotationTransform(i);
	}
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	glBindBuffer(GL_UNIFORM_BUFFER, boundingSphereBuffer);
	Bounds* bptr = reinterpret_cast<Bounds*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, length * sizeof(Bounds), bufMask));
	for (size_t i = 0; i < length; i++)
	{
		bptr[i] = fields->getFieldBounds(i);
	}
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	//recalculate AABB for dirty objects & add them to update list
	WorldFieldUpdateParams params;
	glm::vec3 worldOrigin = glm::vec3(0, 20, 0);
	glm::vec3 maxDist = glm::vec3(86, 43, 86);
	for (size_t i = 0; i < length; ++i)
	{
		if (fields->objectNeedsUpdate(i, params, m_worldShader->dispatchSize, worldOrigin, maxDist))
		{
			updateList.push(params);
			std::cout << params.size.x << " " << params.size.y << " " << params.size.z << std::endl;
		}
	}
}

float µTec::Renderer::GetPixelExtents(glm::mat4 VP, float z)
{
	glm::vec4 topLeft = glm::vec4(1, 1, z, 1);
	glm::vec4 topRight = glm::vec4(-1, 1, z, 1);
	glm::vec4 tTopLeft = VP*topLeft;
	glm::vec4 tTopRight = VP*topRight;
	glm::vec3 wsTopLeft = glm::vec3(tTopLeft.x, tTopLeft.y, tTopLeft.z) / tTopLeft.w;
	glm::vec3 wsTopRight = glm::vec3(tTopRight.x, tTopRight.y, tTopRight.z) / tTopRight.w;
	float wsDist = glm::length(wsTopLeft - wsTopRight);
	return wsDist / mainCamera->WINDOW_WIDTH;
}

void Renderer::drawRayMarching(Renderable* currentObject)
{
	if (effectsTime==0.0)
		effectsTime = GameTime::timeFromStartup;

	glm::mat4 VP = glm::inverse(cachedVP);
	
	UpdateUniforms(VP);

	UpdateTransformBuffer();



	m_fcShader->activate();
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, paramBuffer);
	glDispatchCompute(30, 18, 1);

	UpdateWorldDistanceField();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	ComputeHits(VP);

	ComputeEffects();

	RenderLighting(VP, currentObject);

	//m_factory->CheckGlError("catchall");
}
