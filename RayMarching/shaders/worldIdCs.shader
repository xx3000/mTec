// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=4,local_size_y=4,local_size_z=4) in;

#include "constants.inc"

uniform isampler3D worldIds;
restrict writeonly uniform iimage3D processedIds;

uniform ivec3 offset;
uniform ivec3 totalGroups;

void main()                                                                         
 {
	ivec3 storePos = ivec3(gl_GlobalInvocationID.xyz+offset*gl_WorkGroupSize);
	vec3 uvDist = 1.0/ vec3(totalGroups*gl_WorkGroupSize);
	vec3 uv = (vec3(storePos)) *uvDist;

	vec3 offset = (uvDist)*2.0;

	int vals[26];
	
	vals[0] = texture(worldIds,uv+vec3(offset.x,-offset.y,-offset.z)).x;
	vals[1] = texture(worldIds,uv+vec3(offset.x,-offset.y,offset.z)).x;
	vals[2] = texture(worldIds,uv+vec3(offset.x,offset.y,-offset.z)).x;
	vals[3] = texture(worldIds,uv+vec3(offset.x,offset.y,offset.z)).x;
	vals[4] = texture(worldIds,uv+vec3(-offset.x,-offset.y,-offset.z)).x;
	vals[5] = texture(worldIds,uv+vec3(-offset.x,-offset.y,offset.z)).x;
	vals[6] = texture(worldIds,uv+vec3(-offset.x,offset.y,-offset.z)).x;
	vals[7] = texture(worldIds,uv+vec3(-offset.x,offset.y,offset.z)).x;
	
	vals[8] =  texture(worldIds,uv+vec3(0,-offset.y,-offset.z)).x;
	vals[9] =  texture(worldIds,uv+vec3(0,-offset.y,offset.z)).x;
	vals[10] = texture(worldIds,uv+vec3(0,offset.y,-offset.z)).x;
	vals[11] = texture(worldIds,uv+vec3(0,offset.y,offset.z)).x;
	vals[12] = texture(worldIds,uv+vec3(offset.x,0,-offset.z)).x;
	vals[13] = texture(worldIds,uv+vec3(offset.x,0,offset.z)).x;
	vals[14] = texture(worldIds,uv+vec3(-offset.x,0,-offset.z)).x;
	vals[15] = texture(worldIds,uv+vec3(-offset.x,0,offset.z)).x;
	vals[16] = texture(worldIds,uv+vec3(-offset.x,-offset.y,0)).x;
	vals[17] = texture(worldIds,uv+vec3(-offset.x,offset.y,0)).x;
	vals[18] = texture(worldIds,uv+vec3(offset.x,-offset.y,0)).x;
	vals[19] = texture(worldIds,uv+vec3(offset.x,offset.y,0)).x;
	
	
	vals[20] = texture(worldIds,uv+vec3(-offset.x,0,0)).x;
	vals[21] = texture(worldIds,uv+vec3(offset.x,0,0)).x;
	vals[22] = texture(worldIds,uv+vec3(0,offset.y,0)).x;
	vals[23] = texture(worldIds,uv+vec3(0,-offset.y,0)).x;
	vals[24] = texture(worldIds,uv+vec3(0,0,offset.z)).x;
	vals[25] = texture(worldIds,uv+vec3(0,0,-offset.z)).x;
	
	bool equal =true;
	for(int i =1; i < 26 ; ++i )
	{
		if(vals[0] != vals[i])
		{
		equal=false;
		break;
		}
	}
	int val = equal ? vals[0] : 0;
	imageStore(processedIds, storePos, ivec4(val,1,1,1));
	//imageStore(processedIds, storePos, ivec4(1,1,1,1));
	
	return;
 }