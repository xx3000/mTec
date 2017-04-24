// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=8,local_size_y=8,local_size_z=1) in;

restrict writeonly uniform image2D posBuffer;
restrict writeonly uniform image2D normalBuffer;
restrict writeonly uniform image2D ThickBuffer;
restrict writeonly uniform iimage2D IdBuffer;





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

layout(std140, binding = 7) uniform rotbuffer {
	mat4 rotationTrans[MAX_OBJECT_COUNT];
};

layout(std430, binding = 11) buffer idbuffer {
	int ids[];
};

 uniform vec4 time;
 uniform vec4 sh_ao_tl_sd;



#include "distanceFunctions.inc"
#include "spheretracing.inc"
#include "helper.inc"
#include "conetracing.inc"
#include "aoTracing.inc"

vec3 calcNormal( vec3 pos, float type)
{
	vec3  eps = vec3( 0.1, 0.0,0.0 );

	if(type == PLANE)
	{
	return vec3(0,-1,0);

	}
	else if (type > 0)
	{
	int id = int(type);
	return normalize( vec3( sdfSpec(pos-eps.xyz,id).x - sdfSpec(pos+eps.xyz,id).x,
							sdfSpec(pos-eps.zxy,id).x - sdfSpec(pos+eps.zxy,id).x,
							sdfSpec(pos-eps.zyx,id).x - sdfSpec(pos+eps.zyx,id).x ) );
	}
	else if (type == WALLS || type == COLS)
	{
	return normalize( vec3( room(pos-eps.xyz).x - room(pos+eps.xyz).x,
							room(pos-eps.zxy).x - room(pos+eps.zxy).x,
							room(pos-eps.zyx).x - room(pos+eps.zyx).x ) );
	}
	else if (type == COLS)
	{
	return normalize( vec3( cols(pos-eps.xyz).x - cols(pos+eps.xyz).x,
							cols(pos-eps.zxy).x - cols(pos+eps.zxy).x,
							cols(pos-eps.zyx).x - cols(pos+eps.zyx).x ) );
	}
	else
	{
	return vec3(0,1,0);
	}
  
}

 void main()                                                                         
 {  

	vec4 FragColor;
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = (vec2(gl_GlobalInvocationID.xy)+vec2(0.5,0.5)) / renderDimensions;
	//vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(gl_NumWorkGroups.x*gl_WorkGroupSize.x,gl_NumWorkGroups.y*gl_WorkGroupSize.y);
	
	vec4 tpos =matVP * vec4(uv.x*2.0-1.0,uv.y*2.0-1.0,0, 1.0);
	vec3 WorldPos = tpos.xyz/tpos.w;

	ivec2 groupid = ivec2(gl_WorkGroupID.xy);
	int cullIndex =(int(groupid.x)+int(groupid.y)*int(int(gl_NumWorkGroups.x)))*MAX_OBJECT_COUNT;
	
	vec3 eye = gEyeWorldPos.xyz;
	vec3 rayDirection =normalize(WorldPos - eye );

    // get intersection distance
	TraceResult res;
    res=combinedMarch(eye,rayDirection,cullIndex);

    

	float steps =res.steps /MAX;
	vec3 normal = calcNormal(res.hit,res.sdf);

	float thickness = 0.0;
	if(res.sdf == 4)
	{
		thickness=ThicknessTrace(res.hit,rayDirection,0.075, objects[int(res.sdf)].lightParams.z, 20,int(res.sdf));
	}
	
	imageStore(posBuffer, storePos, vec4(res.distance,0,0,0));
	vec4 encodedNormal = encodeNormal(normal);
	imageStore(normalBuffer, storePos, encodedNormal);
	imageStore(ThickBuffer,storePos,vec4(thickness,0,0,0));
	imageStore(IdBuffer,storePos,ivec4(int(res.sdf),0,0,0));
	return;
 }