// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)

#pragma once

#include "GameLoop.h"
#include <string>
#include "AssetManager.h"

using namespace µTec;

class Game :
	public GameLoop
{
public:
	Game(void);
	~Game(void);
	virtual void init(int* argc, char** argv);

	void setupPrimitives();

	virtual void draw();
	virtual void update();
	void switchTimeSpeed();
	void switchTime();
	void animate();
	void setShadows();
	void setAmbient();
	void setTranslucency();
	void setDebugSteps();
	void ReloadShaderIncludes();
	void ReloadShaderIncludes(const char* name);
private:
	Renderer* m_renderer;
	InputHandler3D* m_inputHandler;
	Camera* m_mainCamera;
	std::string shaderFileLocation;
};


