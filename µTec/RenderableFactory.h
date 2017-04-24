// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#pragma once
#include "RenderingHelperFunctions.h"
#include <vector>
#include "ObjectLoader.h"
#include "TextureLoader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace µTec
{

	struct Dimensions
	{
		Dimensions(size_t width, size_t height, size_t depth) : width(width), height(height),
			depth(depth), _length(width*height*depth)
		{}

		size_t length()
		{
			return _length;
		}
		const size_t width;
		const size_t height;
		const size_t depth;
	private:
		const size_t _length;
	};

struct BufferStorage
{
	GLuint staticVertexBuffer;
	GLuint staticIndexBuffer;
	GLuint dynamicVertexBuffer;
	GLuint dynamicIndexBuffer;
};

struct SDF {
	size_t width, height, depth, dimensions;
	float gridResolution;
	float* data;
};

struct FrameBuffer
{
	GLenum drawBuffer;
	GLuint frameBuffer;
	GLuint renderTexture;
	GLuint depthBuffer;
};

class RenderableFactory
{
public:
	RenderableFactory(void) {
		m_objectLoader= new ObjectLoader();
		m_textureManager= new TextureManager();
	}
	~RenderableFactory(void) {
		delete m_textureManager;
		delete m_objectLoader;
	}

	void deleteImages()
	{
		delete m_textureManager;
		m_textureManager = nullptr;
	}

	void LoadTextures(const char* path)
	{
		m_textureManager->loadFromDir(path);
	}

	void createBindlessTexture(DynamicImage* texture, GLuint64* handle, bool isLinear);

	std::vector<Renderable*> setupRenderablesFromDirectory(const char* objPath, const char* texturePath);

	void addStaticObjectsByDirectory(const char* objPath, const char* texturePath,std::vector<Renderable*>& staticRenderables,BufferStorage& defaultStorage,BufferStorage& adjacentStorage,bool adjacent);

	void addDynamicObjectsByDirectory(const char* objPath, const char* texturePath,std::vector<Renderable*>& dynamicRenderables, BufferStorage& defaultStorage,BufferStorage& adjacentStorage,bool adjacent);

	void addDynamicRenderablesToRenderList( std::vector<Renderable*> &dynamicRenderables, std::vector<Renderable*> &tmpRenderables, BufferStorage& defaultStorage,BufferStorage& adjacentStorage);


		TextureManager* m_textureManager;
		void CheckGlError(std::string desc);
		void bindBufferTexture(size_t width, size_t height, void* data, GLuint* tex, GLint format, GLint intnl, bool linear=false);
		void bind3DBufferTexture(size_t width, size_t height, size_t depth, void* data, GLuint* tex, GLint format, GLint intnl, GLint datatype, bool linear = true);

		void bindTexture(DynamicImage* texture, GLuint* tex, bool isLinear, int level = 0);
		void bindTextureRaw(size_t width, size_t height, unsigned char* data, GLuint* tex, bool isLinear, int level = 0);

		void generateFramebuffer(FrameBuffer& FB, unsigned windowWidth, unsigned windowHeight, GLint internalFormat, GLint format, GLint type);
		void bind3DTexture(Dimensions& dim, float* data, GLuint* tex);
		void bindless3DTexture(Dimensions& dim, float* data, GLuint64* handle);

		template<typename T>
		void generateAndFillBuffer(std::vector<T>& items, GLuint& buffer)
		{
			CheckGlError("Buffer Binding I");
			glDeleteBuffers(1, &buffer);
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, items.size() * sizeof(T), NULL, GL_STATIC_DRAW);
			GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT; // the invalidate makes a big difference when re-writing
			T* ptr = reinterpret_cast<T*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, items.size() * sizeof(T), bufMask));
			memcpy(ptr, &(items[0]), items.size() * sizeof(T));
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			CheckGlError("Buffer Binding IV");
		}
		template<typename T>
		void generateUniformBuffer(T* content,int count, GLuint& buffer, bool exact =false)
		{
			glDeleteBuffers(1, &buffer);
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_UNIFORM_BUFFER, buffer);
			float size = exact ? count : 128;
			glBufferData(GL_UNIFORM_BUFFER, sizeof(T)*size, NULL, GL_STATIC_DRAW);
			GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT; // the invalidate makes a big difference when re-writing
			T* ptr = reinterpret_cast<T*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(T)*count, bufMask));
			CheckGlError("Buffer Binding Params");
			memcpy(ptr, content, count * sizeof(T));
			glUnmapBuffer(GL_UNIFORM_BUFFER);
			
		}

		DynamicImage* loadTexture(const char* name, const char* path);

		void loadAndBindTexture(const char* name, const char* path, GLuint* tex, bool isLinear = false, int level = 0);
		void loadAndGetTextureHandle(const char* name, const char* path, GLuint64* handle, bool isLinear = false);
		Renderable* getStaticPrototypeByName(const char* name);
		void addStaticObjectsFromDirectory(const char* objPath, const char* texturePath);
		void calculateModelMatrix(std::vector<Renderable*> renderables);

		BufferStorage defaultBuffers;
		BufferStorage adjacentBuffers;
private:


	std::vector<Renderable*> staticPrototypes;


	ObjectLoader* m_objectLoader;

	void generateVertexAndIndexBuffers(std::vector<Renderable*> renderables, GLuint* VBO, GLuint* IBO,int MODE);

	void generateIndexBuffer( GLuint* IBO, unsigned int totalFaceCount, std::vector<Renderable*> &renderables );

	void loadAndAssignTextures(std::vector<Renderable*> renderables, const char* texturePath);
};

}
