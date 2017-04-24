// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=4,local_size_y=4,local_size_z=4) in;

#include "constants.inc"
#include "cullingHelper.inc"

layout(std430, binding = 3) buffer fieldbuffer {
	sampler3D fields[];
};

layout(std140, binding = 5) uniform indexbuffer {
	SDF objects[MAX_OBJECT_COUNT];
};

layout(std140, binding = 6) uniform matbuffer {
	mat4 transforms[MAX_OBJECT_COUNT];
};

restrict writeonly uniform image3D worldSdf;
restrict writeonly uniform iimage3D worldSdf2;

uniform ivec3 offset;
uniform ivec3 totalGroups;

int ids[1];

#include "distanceFunctions.inc"

void main()                                                                         
 {
	ivec3 storePos = ivec3(gl_GlobalInvocationID.xyz+offset*gl_WorkGroupSize);
	vec3 uvDist = 1.0/ vec3(totalGroups*gl_WorkGroupSize);
	vec3 uv = (vec3(storePos)+vec3(0.5,0.5,0.5)) *uvDist;
	//uv-=vec3(0.5);
	//uv*=2.0;
	
	
	vec3 eye = vec3(0, 20, 0);

	vec3 maxDist= vec3(86,43,86);
	vec3 origin = eye-(maxDist*0.5);
	vec3 pos = origin+uv*maxDist;
	storePos+=ivec3(2);
	
	//imageStore(worldSdf, storePos, vec4(box(uv).x,1,1,1));
	float dist =combinedSDF(pos);
	imageStore(worldSdf, storePos, vec4(dist,1,1,1)-0.1);
    // imageStore(worldSdf, storePos, vec4(uv.x,1,1,1));
	
	int vals;
	
	vals = int(GetSDF(pos));
	
	//if(dist > 0.0) val = 0;
	imageStore(worldSdf2, storePos, ivec4(vals,1,1,1));
	//imageStore(worldSdf2, storePos, ivec4(int(GetSDF(pos)),1,1,1));
	//imageStore(worldSdf2, storePos, ivec4(1,1,1,1));
	
	return;
 }