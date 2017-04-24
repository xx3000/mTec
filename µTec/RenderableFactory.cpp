// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#include "RenderableFactory.h"
#include <iostream>
#include <fstream>


using namespace µTec;

void RenderableFactory::calculateModelMatrix(std::vector<Renderable*> renderables)
{
	glm::mat4 id, scale, rotate, center;

	Renderable* current;
	for (unsigned int i = 0; i < renderables.size(); ++i)
	{
		current = renderables[i];
		scale = glm::scale(id, current->getScale());
		rotate = glm::toMat4(glm::quat(current->getRotation()));
		center = glm::translate(id, current->getPosition());
		current->model = center*rotate*scale;
	}
}

Renderable* RenderableFactory::getStaticPrototypeByName(const char* name)
{
	for (unsigned int i = 0; i < staticPrototypes.size(); ++i)
	{
		if (strcmp(staticPrototypes[i]->name, name) == 0) return staticPrototypes[i];
	}
	return nullptr;
}

void RenderableFactory::addStaticObjectsFromDirectory(const char* objPath, const char* texturePath)
{
	addStaticObjectsByDirectory(objPath, texturePath, staticPrototypes, defaultBuffers, adjacentBuffers, false);
	calculateModelMatrix(staticPrototypes);
}

void RenderableFactory::CheckGlError(std::string desc)
{
		GLenum e = glGetError();
		if (e != GL_NO_ERROR) {
			fprintf(stderr, "OpenGL error in \"%s\": %s (%d)\n", desc.c_str(), gluErrorString(e), e);

		}
}

void RenderableFactory::bindTexture(DynamicImage* texture, GLuint* tex, bool isLinear, int level)
{
	if (texture != nullptr)
	{
		bindTextureRaw(texture->width, texture->height, &(texture->rawData[0]),tex,isLinear,level);
	}
}

void RenderableFactory::bindTextureRaw(size_t width, size_t height,unsigned char* data, GLuint* tex, bool isLinear, int level)
{
	GLint format = isLinear ? GL_R11F_G11F_B10F : GL_SRGB8_ALPHA8;
	glDeleteTextures(1, tex);
	if (level == 0)
	{
		glGenTextures(1, tex);
	}
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	//CheckGlError("texbind 1");
	GLfloat aniso;
	glEnable(GL_TEXTURE_2D);
	//CheckGlError("texbind 2");
	if (level == 0)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
	//CheckGlError("texbind 3");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //GL_NEAREST = no smoothing
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//CheckGlError("texbind 4");

}

void RenderableFactory::generateFramebuffer(FrameBuffer& FB, unsigned windowWidth, unsigned windowHeight, GLint internalFormat, GLint format, GLint type)
{
	//Delete resources
	glDeleteTextures(1, &FB.renderTexture);
	glDeleteTextures(1, &FB.depthBuffer);
	glDeleteFramebuffers(1, &FB.frameBuffer);

	//generate the new buffer
	glGenFramebuffers(1, &FB.frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, FB.frameBuffer);

	// The texture we're going to render to
	glGenTextures(1, &FB.renderTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture, note the MULTISAMPLE flag.

	glBindTexture(GL_TEXTURE_2D, FB.renderTexture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, windowWidth, windowHeight, 0, format, type, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &FB.depthBuffer);

	glBindTexture(GL_TEXTURE_2D, FB.depthBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);

	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, FB.depthBuffer);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, FB.depthBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FB.depthBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FB.renderTexture, 0);
	CheckGlError("fb 3");
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, FB.depthTexture, 0);

	// Set the list of draw buffers.
	FB.drawBuffer = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &FB.drawBuffer); // "1" is the size of DrawBuffers

	GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (error != GL_FRAMEBUFFER_COMPLETE)
	{
		exit(1);
	}
}

std::vector<Renderable*> RenderableFactory::setupRenderablesFromDirectory( const char* objPath, const char* texturePath )
{
	std::vector<Renderable*> renderables=m_objectLoader->loadObjectsFromDir(objPath);
	loadAndAssignTextures(renderables,texturePath);
	return renderables;
}

void RenderableFactory::addStaticObjectsByDirectory( const char* objPath, const char* texturePath,std::vector<Renderable*>& staticRenderables, BufferStorage& defaultStorage,BufferStorage& adjacentStorage,bool adjacent)
{
	std::vector<Renderable*> tmpRenderables = setupRenderablesFromDirectory(objPath,texturePath);
	staticRenderables.insert(staticRenderables.end(), tmpRenderables.begin(), tmpRenderables.end());
	generateVertexAndIndexBuffers(staticRenderables,&defaultStorage.staticVertexBuffer,&defaultStorage.staticIndexBuffer,GL_STATIC_DRAW);
	//generateAdjacentVertexAndIndexBuffers(staticRenderables,&adjacentStorage.staticVertexBuffer,&adjacentStorage.staticIndexBuffer,GL_STATIC_DRAW);

}

void RenderableFactory::addDynamicObjectsByDirectory( const char* objPath, const char* texturePath,std::vector<Renderable*>& dynamicRenderables, BufferStorage& defaultStorage,BufferStorage& adjacentStorage,bool adjacent)
{
	std::vector<Renderable*> tmpRenderables = setupRenderablesFromDirectory(objPath,texturePath);
	addDynamicRenderablesToRenderList(dynamicRenderables, tmpRenderables, defaultStorage, adjacentStorage);

}

void RenderableFactory::generateVertexAndIndexBuffers( std::vector<Renderable*> renderables, GLuint* VBO, GLuint* IBO, int MODE)
{
	unsigned int totalVertexCount=0;
	unsigned int totalIndexCount=0;
	for(unsigned int i=0; i< renderables.size();++i)
	{
		Renderable* current= renderables[i];
		current->defaultLayout->indexOffset=totalIndexCount;
		current->defaultLayout->vertexOffset=totalVertexCount;
		totalVertexCount+=current->defaultLayout->vertexCount;
		totalIndexCount+=current->defaultLayout->indexCount;
	}

	glGenBuffers(1, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*totalVertexCount, NULL, MODE);

	//if(adjacent)generateAdjacentIndexBuffer(IBO, totalFaceCount, adjacent, renderables);
	generateIndexBuffer(IBO, totalIndexCount, renderables);

}

DynamicImage* RenderableFactory::loadTexture(const char* name, const char* path)
{
	std::string sName = name;
	sName.append(".png");
	DynamicImage* image = m_textureManager->getTextureByName(sName.c_str());
	if (image == nullptr)
	{
			image = m_textureManager->getTextureByName("tiny");
	}
	return image;
}

void RenderableFactory::loadAndAssignTextures( std::vector<Renderable*> renderables, const char* texturePath )
{
	for(unsigned int i=0; i< renderables.size();++i)
	{
		Renderable* renderable = renderables[i];
		std::string sName= renderable->name;
		sName.append(".png");
		DynamicImage* image= m_textureManager->getTextureByName(sName.c_str());
		if(image == nullptr)
		{
			image = m_textureManager->getTextureByName("tiny");
		}
		renderable->texture=image;

		// load normal map if available
		sName=renderable->name;
		sName.append("_NORM.png");
		image= m_textureManager->getTextureByName(sName.c_str());
		renderable->normalMap=image;

	}
}

void RenderableFactory::generateIndexBuffer( GLuint* IBO, unsigned int totalIndexCount, std::vector<Renderable*> &renderables )
{
	glGenBuffers(1, IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*totalIndexCount, NULL, GL_STATIC_DRAW);

	for(unsigned int i=0; i< renderables.size();++i)
	{
		Renderable* current= renderables[i];
		glBufferSubData(GL_ARRAY_BUFFER,  sizeof(Vertex)*current->defaultLayout->vertexOffset,  sizeof(Vertex)*current->defaultLayout->vertexCount,  current->defaultLayout->vertices);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(unsigned int)*current->defaultLayout->indexOffset,  sizeof(unsigned int)*current->defaultLayout->indexCount,  current->defaultLayout->indices);
	}
}

void RenderableFactory::addDynamicRenderablesToRenderList( std::vector<Renderable*> &dynamicRenderables, std::vector<Renderable*> &tmpRenderables, BufferStorage& defaultStorage,BufferStorage& adjacentStorage)
{
	dynamicRenderables.insert(dynamicRenderables.end(), tmpRenderables.begin(), tmpRenderables.end());
	generateVertexAndIndexBuffers(dynamicRenderables,&defaultStorage.dynamicVertexBuffer,&defaultStorage.dynamicIndexBuffer,GL_DYNAMIC_DRAW);
}

void µTec::RenderableFactory::loadAndBindTexture(const char* name, const char* path, GLuint* tex, bool isLinear,int level)
{
	DynamicImage* img = loadTexture(name, path);
	bindTexture(img, tex,isLinear,level);
}

void µTec::RenderableFactory::loadAndGetTextureHandle(const char* name, const char* path, GLuint64* handle, bool isLinear)
{
	DynamicImage* img = loadTexture(name, path);
	createBindlessTexture(img, handle,isLinear);
}

void µTec::RenderableFactory::createBindlessTexture(DynamicImage* texture, GLuint64* handle, bool isLinear)
{
	GLuint texId;
	glActiveTexture(GL_TEXTURE0);
	if (texture != nullptr)
	{
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		GLint format = isLinear ? GL_RGBA8 : GL_SRGB8_ALPHA8;
		glTexImage2D(GL_TEXTURE_2D, 0, format, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(texture->rawData[0]));
		CheckGlError("bindless 1");
		
		glEnable(GL_TEXTURE_2D);
		
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST = no smoothing
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_REPEAT);
		CheckGlError("bindless 2");
		
		*handle = glGetTextureHandleARB(texId);
		CheckGlError("bindless 3");
		glMakeTextureHandleResidentARB(*handle);
		CheckGlError("bindless 4");

	}
}

void µTec::RenderableFactory::bindBufferTexture(size_t width, size_t height, void* data, GLuint* tex, GLint format, GLint intnl, bool linear/*=false*/)
{
	glDeleteTextures(1, tex);

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	GLint type = GL_FLOAT;
	if (format == GL_R8I)
		type = GL_INT;
	if (format == GL_RGBA8)
		type = GL_UNSIGNED_BYTE;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, intnl, type, data);
	CheckGlError("compute texbind 1");
	glEnable(GL_TEXTURE_2D);
	
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	if (format == GL_R8I )
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_NEAREST = no smoothing
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else if (linear)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //GL_NEAREST = no smoothing
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); //GL_NEAREST = no smoothing
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	CheckGlError("compute texbind 3");
}

void µTec::RenderableFactory::bind3DBufferTexture(size_t width, size_t height, size_t depth, void* data, GLuint* texId, GLint format, GLint intnl,GLint datatype, bool linear)
{
	//GLuint texId;
	glDeleteTextures(1, texId);
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, texId);
	glBindTexture(GL_TEXTURE_3D, *texId);

	glTexImage3D(GL_TEXTURE_3D, 0, format, width, height, depth, 0, intnl, datatype, nullptr);
	CheckGlError("compute texbind 0");
	//glTexStorage3D(GL_TEXTURE_3D, 5, GL_R32F, width, height, depth);
	//glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, intnl, datatype, data);
	glEnable(GL_TEXTURE_3D);
	glClearTexImage(*texId, 0, intnl, datatype, NULL);

	CheckGlError("compute texbind 1");
	
	if (linear)
	{
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST = no smoothing
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_NEAREST = no smoothing
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	CheckGlError("compute texbind 3");
	//*handle1 = glGetTextureHandleARB(texId);
	//CheckGlError("bindless3d 3");
	//glMakeTextureHandleResidentARB(*handle1);
	//CheckGlError("bindless3d 4");

	//*handle2 = glGetImageHandleARB(texId, 0, false, 0, format);
	//CheckGlError("bindless3d 3");
	//glMakeImageHandleResidentARB(*handle2, GL_WRITE_ONLY);

}

void µTec::RenderableFactory::bind3DTexture(Dimensions& dim, float* data, GLuint* tex)
{
	glDeleteTextures(1, tex);
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_3D, *tex);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, sdf.width, sdf.height, sdf.depth, 0, GL_RED, GL_FLOAT, sdf.data);
	glTexStorage3D(GL_TEXTURE_3D, 5, GL_R32F, dim.width, dim.height,dim.depth);
	glTexSubImage3D(GL_TEXTURE_3D,0,0, 0,0, dim.width, dim.height, dim.depth, GL_RED, GL_FLOAT, data);
	CheckGlError("1");
	
	//glEnable(GL_TEXTURE_3D);

	glGenerateMipmap(GL_TEXTURE_3D);
	CheckGlError("2");
	//GLfloat aniso;
	//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	//glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //GL_NEAREST = no smoothing
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	CheckGlError("3");
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	CheckGlError("4");
}

void µTec::RenderableFactory::bindless3DTexture(Dimensions& dim, float* data, GLuint64* handle)
{
	GLuint texId;
	glActiveTexture(GL_TEXTURE0);
	if (data != nullptr)
	{
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_3D, texId);
		glTexStorage3D(GL_TEXTURE_3D, 5, GL_R16F, dim.width, dim.height, dim.depth);
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, dim.width, dim.height, dim.depth, GL_RED, GL_FLOAT, data);
		CheckGlError("bindless3d 1");

		glEnable(GL_TEXTURE_3D);

		
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //GL_NEAREST = no smoothing
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_3D);
		CheckGlError("bindless3d 2");

		*handle = glGetTextureHandleARB(texId);
		CheckGlError("bindless3d 3");
		glMakeTextureHandleResidentARB(*handle);
		CheckGlError("bindless3d 4");

	}
}


