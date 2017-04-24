// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#include "GlobalShader.h"

using namespace µTec;


void RayMarchingComputeShader::SetVP(const glm::mat4& WVP)
{
	glUniformMatrix4fv(gVPLocation, 1, GL_FALSE, &WVP[0][0]);
}

void µTec::RayMarchingComputeShader::SetTime(const glm::vec4& t)
{
	glUniform4f(time, t.x, t.y, t.z, t.w);
}

void µTec::RayMarchingComputeShader::SetObjectLength(const int& len)
{
	glUniform1i(objLength, len);
}

void µTec::RayMarchingComputeShader::ToggleEffects(const glm::vec4& eff)
{
	glUniform4f(effectToggle, eff.x, eff.y, eff.z, eff.w);
}


void µTec::RayMarchingComputeShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}

NullShader::NullShader(void) : BaseShader("nullVs.shader", "nullFs.shader")
{
	bindUniforms();
}

NullShader::~NullShader( void )
{

}

void NullShader::bindUniforms()
{
	activate();
	gWVPLocation = glGetUniformLocation(shaderID, "gWVP");
}

void NullShader::SetWVP( const glm::mat4& WVP )
{
	glUniformMatrix4fv(gWVPLocation, 1, GL_FALSE, &WVP[0][0]);
}



FXAAShader::FXAAShader(void) : BaseShader("fxaaVs.shader","fxaaFs.shader")
{
	bindUniforms();
}


FXAAShader::~FXAAShader(void)
{
}

void FXAAShader::bindUniforms()
{
	activate();
	Texture0 = glGetUniformLocation(shaderID, "textureSampler");
	gWVPLocation = glGetUniformLocation(shaderID, "gWVP");
}

void FXAAShader::SetWVP( const glm::mat4& WVP )
{
	glUniformMatrix4fv(gWVPLocation, 1, GL_FALSE, &WVP[0][0]);
}

µTec::ShadingShader::ShadingShader(void) : BaseShader("defaultVs.shader", "shadingFs.shader")
{
	bindUniforms();
}

µTec::ShadingShader::~ShadingShader(void)
{
	
}

void µTec::ShadingShader::SetVP(const glm::mat4& WVP)
{
	glUniformMatrix4fv(gVPLocation, 1, GL_FALSE, &WVP[0][0]);
}

void µTec::ShadingShader::bindUniforms()
{
	activate();
	positions = glGetUniformLocation(shaderID, "positions");
	normals = glGetUniformLocation(shaderID, "normals");
	thick = glGetUniformLocation(shaderID, "thick");
	ids = glGetUniformLocation(shaderID, "ids");
	shadows = glGetUniformLocation(shaderID, "shadows");
	ao = glGetUniformLocation(shaderID, "ao");
	sss = glGetUniformLocation(shaderID, "sss");
	gVPLocation = glGetUniformLocation(shaderID, "gVP");
	reflections = glGetUniformLocation(shaderID, "reflections");
}

void µTec::AoComputeShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}

void µTec::AoComputeShader::SetVP(const glm::mat4& WVP)
{
	glUniformMatrix4fv(gVPLocation, 1, GL_FALSE, &WVP[0][0]);
}

void µTec::SssComputeShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}

void µTec::SssComputeShader::SetVP(const glm::mat4& WVP)
{
	glUniformMatrix4fv(gVPLocation, 1, GL_FALSE, &WVP[0][0]);
}

void µTec::FieldCullingComputeShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}

void µTec::FieldCullingComputeShader::SetConeAngles(const glm::vec3& angles)
{
	glUniform3f(coneAngle, angles.x, angles.y, angles.z);
}

void µTec::AoFieldCullingShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}

void µTec::ShadowsComputeShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}

void µTec::UpsamplingShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}

µTec::ReprojectionShader::ReprojectionShader(void) : BaseShader("defaultVs.shader", "reprojectionFs.shader")
{
	bindUniforms();
}

µTec::ReprojectionShader::~ReprojectionShader(void)
{

}

void µTec::ReprojectionShader::SetVP(const glm::mat4& WVP)
{
	glUniformMatrix4fv(gVPLocation, 1, GL_FALSE, &WVP[0][0]);
}

void µTec::ReprojectionShader::bindUniforms()
{
	activate();
	frame = glGetUniformLocation(shaderID, "frame");
	pastHistory = glGetUniformLocation(shaderID, "pastHistory");
	newHistory = glGetUniformLocation(shaderID, "newHistory");
	gVPLocation = glGetUniformLocation(shaderID, "gVP");
	positions = glGetUniformLocation(shaderID, "positions");
	velocity = glGetUniformLocation(shaderID, "velocity");
}

void µTec::VelocityComputeShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}

void µTec::VelocityComputeShader::SetHistoryVP(const glm::mat4& VP)
{
	glUniformMatrix4fv(historyVP, 1, GL_FALSE, &VP[0][0]);
}

void µTec::ShadowFieldCullingShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}

void µTec::WorldSdfShader::SetGroups(const glm::ivec3 size)
{
	dispatchSize = size;
	glUniform3i(totalGroups, size.x, size.y, size.z);
}

void µTec::WorldSdfShader::SetOffset(const glm::ivec3 size)
{
	glUniform3i(offset, size.x, size.y, size.z);
}

void µTec::WorldIdShader::SetGroups(const glm::ivec3 size)
{
	dispatchSize = size;
	glUniform3i(totalGroups, size.x, size.y, size.z);
}

void µTec::WorldIdShader::SetOffset(const glm::ivec3 size)
{
	glUniform3i(offset, size.x, size.y, size.z);
}

µTec::BlockMarchingShader::BlockMarchingShader(void) : BaseShader("baseVs.shader", "blockMarchingFs.shader")
{
	bindUniforms();
}

µTec::BlockMarchingShader::~BlockMarchingShader(void)
{

}

void µTec::BlockMarchingShader::SetVP(const glm::mat4& VP)
{
	glUniformMatrix4fv(gVPLocation, 1, GL_FALSE, &VP[0][0]);
}

void µTec::BlockMarchingShader::SetWorld(const glm::mat4& W)
{
	glUniformMatrix4fv(wLocation, 1, GL_FALSE, &W[0][0]);
}

void µTec::BlockMarchingShader::SetInverseWorld(const glm::mat4& W)
{
	glUniformMatrix4fv(invWorld, 1, GL_FALSE, &W[0][0]);
}

void µTec::BlockMarchingShader::SetIndex(int id)
{
	glUniform1i(index, id);
}

void µTec::BlockMarchingShader::SetSDF(GLuint64 handle)
{
	glUniformHandleui64ARB(field, handle);
}

void µTec::BlockMarchingShader::bindUniforms()
{
	activate();
	gVPLocation = glGetUniformLocation(shaderID, "gVP");
	wLocation = glGetUniformLocation(shaderID, "gWorld");
	index = glGetUniformLocation(shaderID, "index");
	field = glGetUniformLocation(shaderID, "field");
	invWorld = glGetUniformLocation(shaderID, "invWorld");
}

µTec::ImplicitBlockMarchingShader::ImplicitBlockMarchingShader(void) : BaseShader("defaultVs.shader", "implicitBlockMarchingFs.shader")
{
	bindUniforms();
}

µTec::ImplicitBlockMarchingShader::~ImplicitBlockMarchingShader(void)
{

}

void µTec::ImplicitBlockMarchingShader::SetVP(const glm::mat4& VP)
{
	glUniformMatrix4fv(gVPLocation, 1, GL_FALSE, &VP[0][0]);
}

void µTec::ImplicitBlockMarchingShader::SetIndex(int id)
{
	glUniform1i(index, id);
}

void µTec::ImplicitBlockMarchingShader::bindUniforms()
{
	activate();
	gVPLocation = glGetUniformLocation(shaderID, "gVP");
	index = glGetUniformLocation(shaderID, "index");
}

void µTec::ReflectionComputeShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}

void µTec::BlurShader::SetRenderDimensions(const glm::vec2 eff)
{
	glUniform2f(renderDimensions, eff.x, eff.y);
}
