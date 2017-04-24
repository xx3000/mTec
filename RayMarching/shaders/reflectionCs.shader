// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=8,local_size_y=8,local_size_z=1) in;

restrict writeonly uniform image2D refBuffer;


#include "constants.inc"

 uniform sampler2D positions;
 uniform sampler2D normals;
 uniform isampler2D idbuf;

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

layout(std430, binding = 4) buffer texbuffer {
	sampler2D textures[];
};

layout(std430, binding = 8) buffer mapbuffer {
	sampler2D normalMaps[];
};

layout(std430, binding = 9) buffer roughbuffer {
	sampler2D roughnessMaps[];
};

layout(std430, binding = 10) buffer metalbuffer {
	sampler2D metalMaps[];
};

 uniform vec4 time;
 uniform vec4 sh_ao_tl_sd;



#include "distanceFunctions.inc"
#include "spheretracing.inc"
#include "helper.inc"
#include "conetracing.inc"
#include "aoTracing.inc"
#include "lighting.inc"

vec3 calcNormal( vec3 pos, float type)
{
	vec3  eps = vec3( 0.1, 0.0,0.0 );

	if(type == PLANE)
	{
	return vec3(0,-1,0);

	}
	else if (type >= 0)
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
	//vec3 WorldPos = vec3(0.5,0.5,0.5);
	//unsigned int uniqueIndex = gl_GlobalInvocationID.z*gl_NumWorkGroups.x*gl_NumWorkGroups.y +gl_GlobalInvocationID.y*gl_NumWorkGroups.x+gl_GlobalInvocationID.x;	

	ivec2 groupid = ivec2(gl_WorkGroupID.xy);
	//int cullIndex =(int(groupid.x*0.5)+int(groupid.y*0.5)*int(int(gl_NumWorkGroups.x)*0.5))*64;
	int cullIndex =(int(groupid.x)+int(groupid.y)*int(int(gl_NumWorkGroups.x)))*64;

	vec3 eye = gEyeWorldPos.xyz;
	vec3 rayDirection =normalize(WorldPos - eye );
	
	float dist =  texture2D(positions, uv).x;  
	vec4 normal_enc =textureLod(normals, uv,0);                                                                             	
	vec3 normal = decodeNormal(normal_enc.xy);

	int iid = textureLod(idbuf, uv,0).x; 

	vec3 col=vec3(0);
	if(iid == 4)
	{
	vec3 pos = eye+rayDirection*dist;
	rayDirection = reflect(rayDirection,normal);
	
	//pos=eye;

	

    // get intersection distance
	TraceResult res;
    res=worldMarch(pos,rayDirection);

	float steps =res.steps /MAX;
	normal = calcNormal(res.hit,res.sdf);

	float id = res.sdf;

	//imageStore(refBuffer, storePos, vec4(vec3(res.distance/5.0),1));
	//return;

	if(id == SKY)
	{
		col = vec3(0.01,0.01,0.01);
	}
	else if(id == SUN)
	{
		col = vec3(1,1,1);

	}
	else
	{
		
		pos=res.hit;
		int ix =int(id);
		vec3 transP = pos;
		vec3 rotNormal = normal;

		if(id > 0 )
		{	
			mat4 tranf =transforms[objects[ix].transId];
			tranf[0][3]=0;
			transP = (tranf*vec4(pos, 1.0)).xyz;
			rotNormal = (rotationTrans[objects[ix].transId]*vec4(normal, 1.0)).xyz;
		
		}

		if(ix <= 0) ix+=OFFSET;
		int offid=ix;

		float scale = objects[offid].lightParams.w;
		col= triplanarMapping(textures[objects[offid].texId],rotNormal,transP,scale);
		normal = normalMapping(normalMaps[objects[offid].texId],normal,transP,scale);

		float roughness = 0.0;
		float metal = 0.0;
		if(varParams.x==0.0)
		{
			roughness = triplanarMapping(roughnessMaps[objects[offid].texId],rotNormal,transP,scale).r;
			metal = triplanarMapping(metalMaps[objects[offid].texId],rotNormal,transP,scale).r;
		}

		vec3 mSun=normalize(pos-lightPos);
		vec3 reflection= vec3(1);
		col=shadingPBR(col,pos,normal, 0.0, rayDirection , mSun, 1.0,1.0,max(int(id),0),roughness,metal,reflection);
	
	}
	}

	imageStore(refBuffer, storePos, vec4(col,1));
	
	return;
 }