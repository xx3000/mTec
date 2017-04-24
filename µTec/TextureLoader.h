// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once

#include "lodepng.h"
#include "dirent.h"
#include "Vertex.h"
#include <map>
#include <string>
#include <iostream>

namespace µTec
{

class TextureManager
{
public:
	TextureManager();
	TextureManager(const TextureManager& other);
	~TextureManager(void);

	bool loadFromDir(const char* dir);
	bool loadFromFile(const char* file);
	const std::map<std::string,DynamicImage>* getTextures() const;
	DynamicImage* getTextureByName(const char* name) const;

private:
	std::map<std::string,DynamicImage>* textureMap;
};



}