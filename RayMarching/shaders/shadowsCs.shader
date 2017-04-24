// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=8,local_size_y=8,local_size_z=1) in;

restrict writeonly uniform image2D shadowBuffer;
 uniform sampler2D positions;
 uniform sampler2D normals;

  #include "constants.inc"
  
 layout(std430, binding = 3) buffer fieldbuffer {
	sampler3D fields[];
};

layout(std140, binding = 5) uniform indexbuffer {
	SDF objects[MAX_OBJECT_COUNT];
};

layout(std140, binding = 6) uniform matbuffer {
	mat4 transforms[MAX_OBJECT_COUNT];
};

layout(std430, binding = 13) buffer culledbuffer {
	int ids[];
};

#include "distanceFunctions.inc"

#include "conetracing.inc"


void main()                                                                         
 {
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	vec2 totalSize =vec2(gl_NumWorkGroups.x*gl_WorkGroupSize.x,gl_NumWorkGroups.y*gl_WorkGroupSize.y);
	vec2 uv = (vec2(gl_GlobalInvocationID.xy)+vec2(0.5,0.5)) / renderDimensions;
	vec4 tpos =matVP * vec4(uv.x*2.0-1.0,uv.y*2.0-1.0,0, 1.0);
	vec3 WorldPos = tpos.xyz/tpos.w;

	float dist =  textureLod(positions, uv,1).x;                                                                               	
	vec3 normal = decodeNormal(textureLod(normals, uv,0).xy);

	vec3 eye = gEyeWorldPos.xyz;
	vec3 rayDirection =normalize(WorldPos - eye );
	vec3 pos = eye+rayDirection*dist;
	vec3 mSun=normalize(pos-lightPos);
	vec3 dir = -mSun;

	int baseIndex=int(int(storePos.x*0.5)+int(storePos.y*0.5)*int(float(gl_NumWorkGroups.x*gl_WorkGroupSize.x)*0.5))*MAX_OBJECT_COUNT;

	
	float shadow = 1.0;
	{
		shadow = SimpleConeTrace(pos,dir,rayDirection,50.0,0.1,40.0,0.001,6.0,0.1,baseIndex);
	}
	imageStore(shadowBuffer, storePos, vec4(shadow,0,0,0));
 }