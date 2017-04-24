// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)

#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=8,local_size_y=8,local_size_z=1) in;

restrict writeonly uniform image2D aoBuffer;
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
#include "aoTracing.inc"


void main()                                                                         
 {
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = (vec2(gl_GlobalInvocationID.xy)+vec2(0.5,0.5)) / renderDimensions;
	ivec2 groupid = ivec2(vec2(gl_WorkGroupID.xy)/4.0);
	uint offset = gl_NumWorkGroups.x*gl_WorkGroupSize.x*gl_NumWorkGroups.y*gl_WorkGroupSize.y*MAX_OBJECT_COUNT;
	int baseIndex=(storePos.x+(storePos.y*int(gl_NumWorkGroups.x*gl_WorkGroupSize.x)))*MAX_OBJECT_COUNT+int(offset);
	
	vec4 tpos =matVP * vec4(uv.x*2.0-1.0,uv.y*2.0-1.0,0, 1.0);
	vec3 WorldPos = tpos.xyz/tpos.w;

	float dist =  texture2D(positions, uv).x;  
	vec4 normal_id_thickness =texture(normals, uv);                                                                             	
	vec3 normal = decodeNormal(normal_id_thickness.xy);

	vec3 eye = gEyeWorldPos.xyz;
	vec3 rayDirection =normalize(WorldPos - eye );
	vec3 pos = eye+rayDirection*dist;

	float occlusion = AmbientOcclude(pos,normal*-1,baseIndex);
	
	imageStore(aoBuffer, storePos, vec4(occlusion,0,0,0));
 }