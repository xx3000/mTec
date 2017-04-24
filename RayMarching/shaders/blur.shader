// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)


#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=8,local_size_y=8,local_size_z=1) in;

restrict writeonly uniform image2D blurredBuffer;

uniform sampler2D colors;

#include "constants.inc"

void main()                                                                         
 {
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	vec2 uvDiff = 1.0/ renderDimensions;

	vec2 uvTL = (vec2(gl_GlobalInvocationID.xy) ) * uvDiff;
	vec2 halfDist = uvDiff*0.5;
	vec2 uv = uvTL+halfDist;
	
	vec3 blurred = vec3(0);

	float weight = 	0.195346;

	blurred+= texture(colors,uv).xyz*weight;
	
	weight = 	0.123317;

	blurred+= texture(colors,uv+vec2(uvDiff.x,0)).xyz*weight;
	blurred+= texture(colors,uv+vec2(-uvDiff.x,0)).xyz*weight;
	blurred+= texture(colors,uv+vec2(0,uvDiff.y)).xyz*weight;
	blurred+= texture(colors,uv+vec2(0,-uvDiff.y)).xyz*weight;

	weight = 	0.077847;
	blurred+= texture(colors,uv+vec2(uvDiff.x,uvDiff.y)).xyz*weight;
	blurred+= texture(colors,uv+vec2(-uvDiff.x,uvDiff.y)).xyz*weight;
	blurred+= texture(colors,uv+vec2(uvDiff.x,-uvDiff.y)).xyz*weight;
	blurred+= texture(colors,uv+vec2(-uvDiff.x,-uvDiff.y)).xyz*weight;
	
	//imageStore(blurredBuffer, storePos, vec4(blurred,0));
	imageStore(blurredBuffer, storePos, vec4(texture(colors,uv).xyz,0));

 }