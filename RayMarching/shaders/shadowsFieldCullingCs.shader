// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=8,local_size_y=8,local_size_z=1) in;

#include "constants.inc"
#include "cullingHelper.inc"

struct Bound {
 vec4 sphere;
 vec4 forward;
 vec4 up;
 vec4 right;
 };

layout(std140, binding = 10) uniform indexbuffer {
	Bound bounds[MAX_OBJECT_COUNT];
};

layout(std430, binding = 13) buffer fieldbuffer {
	int ids[];
};

uniform sampler2D positions;

void main()                                                                         
 {
 
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = (vec2(gl_GlobalInvocationID.xy)+vec2(0.5,0.5)) / renderDimensions;
	
	int baseIndex=(storePos.x+(storePos.y*int(gl_NumWorkGroups.x*gl_WorkGroupSize.x)))*MAX_OBJECT_COUNT;
	uint offset = gl_NumWorkGroups.x*gl_WorkGroupSize.x*gl_NumWorkGroups.y*gl_WorkGroupSize.y*MAX_OBJECT_COUNT;
	vec4 tpos =matVP * vec4(uv.x*2.0-1.0,uv.y*2.0-1.0,0, 1.0);
	vec3 WorldPos = tpos.xyz/tpos.w;
	vec2 pixelDim = (1.0/renderDimensions)*0.5;
	
	float dist =  textureLod(positions, uv,1).x;                                                                              	
	vec3 eye = gEyeWorldPos.xyz;
	vec3 rayDirection =normalize(WorldPos - eye );
	vec3 pos = eye+rayDirection*dist;
	vec3 mSun=lightPos-pos;
	vec3 dir = normalize(mSun);

	int currentIndex=1;
	int aoIndex=1;
	vec4 posSphere = vec4(pos,1.0);

	for(int i = 1; i < objectLength ; ++i)
	{
	vec4 newSphere = bounds[i].sphere;
	newSphere.w+=0.2;
		if(intersectsNR(newSphere,pos,dir,mSun))
		{
			ids[baseIndex+currentIndex]=i;
			++currentIndex;
		}
		if(intersectsSphere(bounds[i].sphere,posSphere))
		{
			ids[baseIndex+aoIndex+offset]=i;
			++aoIndex;
		}
	}
	
	ids[baseIndex]= currentIndex-1;
	ids[baseIndex+offset]= aoIndex-1;
	//ids[baseIndex]=1;
 }