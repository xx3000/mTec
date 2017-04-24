// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#include "AssetManager.h"
#include "rapidjson/document.h"

using namespace rapidjson;

µTec::AssetManager::AssetManager() : fields(new Fields()), tmp(new TmpFieldHolder()), factory()
{

}

µTec::AssetManager::~AssetManager()
{
	delete[] cacheddata;
}

int µTec::AssetManager::addField(std::string name,bool nofield)
{
	int id = tmp->amount;
	tmp->amount++;
	std::string matpath = MAT_DIR + name + MAT_FILE_ENDING;
	const char* buffer = ReadFile(matpath.c_str());
	if (buffer != nullptr)
	{
		Document document;
		document.Parse(buffer);
		bool error = document.HasParseError();
		std::string fs = document["Field"].GetString();
		if (fs != "")
		{
			std::string fieldname = FIELDS_DIR + fs + FIELD_FILE_ENDING;

			ReadSDFFromFile(*tmp, fieldname.c_str(), nofield);
		}
		else
		{
			Dimensions dim(0, 0, 0);
			tmp->dimension.push_back(dim);
			tmp->resolution.push_back(0);
			tmp->data.push_back(0);
		}

		
		std::cout << "SDF " << name.c_str() << " Loaded & Bound" << std::endl;

		tmp->textures.push_back(document["Texture"].GetString());
		const Value& col = document["Color"];
		tmp->colors.push_back(glm::vec4(col[0].GetFloat(), col[1].GetFloat(),col[2].GetFloat(),col[3].GetFloat()));

		const Value& fillCol = document["FillColor"];
		tmp->fillColors.push_back(glm::vec4(fillCol[0].GetFloat(), fillCol[1].GetFloat(), fillCol[2].GetFloat(), fillCol[3].GetFloat()));

		const Value& spec = document["Specular"];
		tmp->lightParams.push_back(glm::vec4(spec[0].GetFloat(), spec[1].GetFloat(), document["Density"].GetFloat(), document["TextureScale"].GetFloat()));


		delete[] buffer;
		return id;

	}
	else
	{
		// no mat file of that name found
		assert(false);
	}

}

int µTec::AssetManager::cloneField(int id, bool useCache)
{
	int retid = tmp->amount;
	tmp->amount++;
	
	Dimensions other = tmp->dimension[id];
	Dimensions dim(other.width, other.height, other.depth);
	tmp->dimension.push_back(dim);
	tmp->resolution.push_back(tmp->resolution[id]);

	if (useCache)
	{
		GLuint64 handle;
		factory.bindless3DTexture(dim, cacheddata, &handle);
		tmp->data.push_back(handle);
	}
	else
	tmp->data.push_back(tmp->data[id]);
	
	std::cout << "SDF Clone Loaded & Bound" << std::endl;

	tmp->textures.push_back(std::string());
	tmp->colors.push_back(tmp->colors[id]);
	tmp->fillColors.push_back(tmp->fillColors[id]);
	tmp->lightParams.push_back(tmp->lightParams[id]);
	return retid;
}

µTec::Fields* µTec::AssetManager::constructFields()
{
	fields->addFields(tmp->amount, tmp->dimension, tmp->resolution, tmp->data,tmp->textures,tmp->colors, tmp->fillColors,tmp->lightParams);

	return fields;
}

const char* µTec::AssetManager::ReadFile(const char* path,bool binary)
{
	std::ios::openmode mode = std::ios::in;
	if (binary)
		mode = std::ios::in | std::ios::binary;
	std::ifstream ifs(path,mode );
	if (!ifs) {
		std::cout << "Cannot open file [" << path << "]" << std::endl;
		return nullptr;
	}
	ifs.seekg(0, ifs.end);
	int length = (int)ifs.tellg();
	ifs.seekg(0, ifs.beg);
	char* buffer = nullptr;
	if (binary)
	{
		buffer = new char[length];
		ifs.read(buffer, length);
	}
	else
	{
		buffer = new char[length+1];
		ifs.read(buffer, length);
		buffer[length] = '\0';
	}
	ifs.close();
	return buffer;
}

void µTec::AssetManager::ReadSDFFromFile(TmpFieldHolder& tmp, const char* path, bool getdata)
{
	const char* buffer = ReadFile(path,true);
	const char* moveBuffer = buffer;

	size_t width, height, depth;
	float resolution;

	memcpy(&width, moveBuffer, sizeof(size_t));
	moveBuffer += sizeof(size_t);
	memcpy(&height, moveBuffer, sizeof(size_t));
	moveBuffer += sizeof(size_t);
	memcpy(&depth, moveBuffer, sizeof(size_t));
	moveBuffer += sizeof(size_t);
	memcpy(&resolution, moveBuffer, sizeof(float));
	moveBuffer += sizeof(float);

	Dimensions dim(width, height, depth);
	float* data = nullptr;
	data = new float[dim.length()];
	memcpy(data, moveBuffer, sizeof(float)*(dim.length()));


	delete[] buffer;

	tmp.dimension.push_back(dim);
	tmp.resolution.push_back(resolution);
	GLuint64 handle;
	factory.bindless3DTexture(dim, data, &handle);
	tmp.data.push_back(handle);
	if (getdata)
		cacheddata = data;
	else
	delete[] data;
}

µTec::AssetManager::TmpFieldHolder::TmpFieldHolder() : amount(0), dimension(), resolution(), data(), textures(), colors()
{

}
