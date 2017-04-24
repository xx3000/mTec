// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once
#include "Fields.h"
#include <iostream>
#include <fstream>
#include "RenderableFactory.h"

namespace µTec
{
	class AssetManager
	{
	public:
		AssetManager();
		~AssetManager();

		int addField(std::string name,bool nofield = false);
		int cloneField(int id,bool useCache = false);

		Fields* constructFields();


	private:

		struct TmpFieldHolder
		{
			TmpFieldHolder();
			int amount;
			std::vector<Dimensions> dimension;
			std::vector<float> resolution;
			std::vector<GLuint64> data;
			std::vector<std::string> textures;
			std::vector<glm::vec4> colors;
			std::vector<glm::vec4> fillColors;
			std::vector<glm::vec4> lightParams;
		};

		Fields* fields;
		TmpFieldHolder* tmp;
		RenderableFactory factory;
		const std::string FIELDS_DIR = "fields\\";
		const std::string MAT_DIR = "materials\\";
		const std::string MAT_FILE_ENDING = ".mat";
		const std::string FIELD_FILE_ENDING = ".sdf";

		float* cacheddata = nullptr;

		const char* ReadFile(const char* path, bool binary = false);

		void ReadSDFFromFile(TmpFieldHolder& tmp, const char* path, bool getdata);
	};
}