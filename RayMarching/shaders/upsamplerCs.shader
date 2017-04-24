// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=8,local_size_y=8,local_size_z=1) in;

restrict writeonly uniform image2D upsampledBuffer;
uniform sampler2D intensities;
uniform sampler2D positions;
uniform sampler2D normals;

#include "constants.inc"

// Spatio-Temporal Upsampling on the GPU
// Herzog et al. 2011

float TriweightKernel(float x, float dist, float falloff)
{
	return pow(max(0.01,1-(x/dist)),falloff);
}

float GetWeight(vec2 uv,float dist,float centerDepth,vec3 centerNormal)
{
	float depth =texture2D(positions, uv).x;
	float depthDiff = abs(depth-centerDepth);
	vec3 normal =decodeNormal(texture2D(normals, uv).xy);
	float normalG =0.1;
	float linearWeight = dist;
	float depthWeight = TriweightKernel(depthDiff*depthDiff,1.0,3.0);
	float normalWeight = TriweightKernel(max(0,1-(dot(normal,centerNormal))),normalG,3.0);	
	return linearWeight *depthWeight*normalWeight ;
}

void GetWeightedIntensity(vec2 uv,float dist,float centerDepth,vec3 centerNormal,inout float filteredInt, inout float weightsTotal)
{
	float intensity = texture2D(intensities, uv).xyz;
	float weight = GetWeight(uv,dist,centerDepth,centerNormal);
	weightsTotal+= weight;
	filteredInt+= intensity*weight;
}

void main()                                                                         
 {
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	vec2 uvDiff = 1.0/ renderDimensions;

	vec2 uvTL = (vec2(gl_GlobalInvocationID.xy) ) * uvDiff;
	vec2 halfDist = uvDiff*0.5;
	vec2 uv = uvTL+halfDist;

	vec4 tpos =matVP * vec4(uv.x*2.0-1.0,uv.y*2.0-1.0,0, 1.0);
	vec3 WorldPos = tpos.xyz/tpos.w;

	float dist =  texture2D(positions, uv).x;  
	vec4 normal_id_thickness =textureLod(normals, uv,0);                                                                             	
	vec3 normal = decodeNormal(normal_id_thickness.xy);
	float id = normal_id_thickness.z;

	float testInt = texture2D(intensities, uv).x;

	float filteredInt = 0;
	float weightsTotal =0;

	float center=1;
	//center*=2.0;
	GetWeightedIntensity(uv,center,dist,normal,filteredInt,weightsTotal);
	halfDist*=2.0;
	float diag=	1;
	//diag*=5.0;
	GetWeightedIntensity(uv+vec2(-uvDiff.x,-uvDiff.y),diag,dist,normal,filteredInt,weightsTotal);
	GetWeightedIntensity(uv+vec2(uvDiff.x,-uvDiff.y),diag,dist,normal,filteredInt,weightsTotal);
	GetWeightedIntensity(uv+vec2(-uvDiff.x,uvDiff.y),diag,dist,normal,filteredInt,weightsTotal);
	GetWeightedIntensity(uv+vec2(uvDiff.x,uvDiff.y),diag,dist,normal,filteredInt,weightsTotal);
	
	float neigh=1;
	//neigh*=5.0;
	GetWeightedIntensity(uv+vec2(-uvDiff.x,0),neigh,dist,normal,filteredInt,weightsTotal);
	GetWeightedIntensity(uv+vec2(uvDiff.x,0),neigh,dist,normal,filteredInt,weightsTotal);
	GetWeightedIntensity(uv+vec2(0,uvDiff.y),neigh,dist,normal,filteredInt,weightsTotal);
	GetWeightedIntensity(uv+vec2(0,-uvDiff.y),neigh,dist,normal,filteredInt,weightsTotal);

	vec2 doubleDist=uvDiff*2.0;

	filteredInt/=weightsTotal; 
	
	imageStore(upsampledBuffer, storePos, vec4(filteredInt,0,0,0));

 }