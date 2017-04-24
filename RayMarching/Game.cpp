// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#include "Game.h"
#include <string>
#include <fstream>
#include <streambuf>

Game::Game()
{


}

Game::~Game()
{
	delete m_renderer;
	delete m_inputHandler;
	delete m_mainCamera;

}

#define SHADER_LOCATION_FILENAME "shaderLocation.txt"

std::string readShaderLocation(std::string filename)
{
	std::ifstream t(filename);
	std::string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
	return str;
}

void Game::init(int* argc, char** argv)
{
	unsigned width = 1920;
	unsigned height = 1080;
	m_inputHandler->breakLoop = false;
	m_mainCamera = new Camera(width, height);
	m_mainCamera->setPosition(glm::vec3(0, 10, 30));
	m_renderer = new Renderer();
	m_renderer->setMainCamera(m_mainCamera);
	m_renderer->init(argc, argv, "RayMarching", width, height, true);
	m_inputHandler = new InputHandler3D();
	m_inputHandler->setMainCamera(m_mainCamera);
	m_inputHandler->bindActions();
	setupPrimitives();
	m_renderer->initShaders();

	shaderFileLocation = readShaderLocation(SHADER_LOCATION_FILENAME);
	shaderFileLocation.erase(shaderFileLocation.find_last_not_of(" \n\r\t") + 1);

	InputEvent callback;

	callback.Bind<Game, &Game::ReloadShaderIncludes>(this);
	m_inputHandler->registerCallback('w', callback);
	callback.Bind<Game, &Game::switchTime>(this);
	m_inputHandler->registerCallback('e', callback);
	callback.Bind<Game, &Game::switchTimeSpeed>(this);
	m_inputHandler->registerCallback('r', callback);
	callback.Bind<Game, &Game::animate>(this);
	m_inputHandler->registerCallback('a', callback);
	callback.Bind<Game, &Game::setShadows>(this);
	m_inputHandler->registerCallback('d', callback);
	callback.Bind<Game, &Game::setAmbient>(this);
	m_inputHandler->registerCallback('f', callback);
	callback.Bind<Game, &Game::setTranslucency>(this);
	m_inputHandler->registerCallback('g', callback);
	callback.Bind<Game, &Game::setDebugSteps>(this);
	m_inputHandler->registerCallback('s', callback);
}

void Game::animate()
{
	m_renderer->animate = !m_renderer->animate;
}

void Game::switchTime()
{
	m_renderer->passTime = !m_renderer->passTime;
}

void Game::switchTimeSpeed()
{
	if (m_renderer->timeSpeed == 1.0)
		m_renderer->timeSpeed = 5.0;
	else
		m_renderer->timeSpeed = 1.0;
}

void Game::setShadows()
{
	if (m_renderer->rm_shadows == 0)
		m_renderer->rm_shadows++;
	else
		m_renderer->rm_shadows--;
}

void Game::setAmbient()
{
	if (m_renderer->shaderParams.y == 0)
		m_renderer->shaderParams.y = 1.0;
	else
		m_renderer->shaderParams.y = 0.0;
}

void Game::setTranslucency()
{
	if (m_renderer->shaderParams.x == 0)
		m_renderer->shaderParams.x=1.0;
	else
		m_renderer->shaderParams.x = 0.0;
}

void Game::setDebugSteps()
{
	if (m_renderer->rm_debugsteps == 0)
		m_renderer->rm_debugsteps++;
	else
		m_renderer->rm_debugsteps--;
}

void Game::draw()
{
	m_renderer->draw();
}

void Game::update()
{
	m_inputHandler->update();
}

void Game::setupPrimitives()
{
	Fields* fields;
	{
		AssetManager assetManager;
		assetManager.addField("default");
		
		assetManager.addField("floor");
		assetManager.addField("room");
		assetManager.addField("pillars");

		size_t dragon = assetManager.addField("dragon");
		size_t teapot = assetManager.addField("teapot");
		std::vector<size_t> chars;
		chars.push_back(assetManager.addField("char"));
		int cloneId = chars[0];

		chars.push_back(assetManager.addField("char"));
		//chars.push_back(assetManager.addField("char"));
		chars.push_back(assetManager.addField("char"));

		for (int i = 0; i < 10; ++i)
			//chars.push_back(assetManager.cloneField(cloneId));
			chars.push_back(assetManager.addField("char"));

		fields = assetManager.constructFields();

		fields->setPosition(dragon, glm::vec3(0, 4.8, 10));
		fields->setScale(dragon, 1);

		fields->setPosition(teapot, glm::vec3(0, 2.4, -15));
		fields->setScale(teapot, 5);

		int width = 7;
		int height = 2;
		for (int i = 0; i < chars.size(); ++i)
		{
			int posZ= i / (width*height);
			int posX = i %(width);
			int posY = (i) % height;
			fields->setPosition(chars[i], glm::vec3(15 - 5 * posX, 2.8 + 12 * posY, -posZ * 4));
			fields->setScale(chars[i], 0.2);

		}
	}

	m_renderer->BindFields(fields);
}

void Game::ReloadShaderIncludes()
{
	ReloadShaderIncludes("rayMarchingCs.shader");
	ReloadShaderIncludes("shadingFs.shader");
	ReloadShaderIncludes("shadowsCs.shader");
	ReloadShaderIncludes("aoCs.shader");
	ReloadShaderIncludes("fieldCompactorCs.shader");
	ReloadShaderIncludes("worldSdfCs.shader");
	ReloadShaderIncludes("worldIdCs.shader");
	ReloadShaderIncludes("upsamplerCs.shader");
	ReloadShaderIncludes("reprojectionFs.shader");
	ReloadShaderIncludes("shadowsFieldCullingCs.shader");
	ReloadShaderIncludes("blockMarchingFs.shader");
	ReloadShaderIncludes("implicitBlockMarchingFs.shader");
	ReloadShaderIncludes("reflectionCs.shader");

	m_renderer->initShaders();
}

void Game::ReloadShaderIncludes(const char* name)
{
	std::string str;
	str.append("gpp\\gpp.exe -o ");
	str.append(name);
	str.append(" \"");
	str.append(shaderFileLocation);
	str.append(name);
	str.append("\"");
	int ret = system(str.c_str());

	
}
