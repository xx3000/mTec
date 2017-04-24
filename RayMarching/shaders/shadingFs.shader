// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#version 430                                                                  
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

 in vec2 TexCoord0;
 in vec3 WorldPos;                                                                  
 
layout(location = 0) out vec4 FragColor;                                                            

 uniform sampler2D positions;
 uniform sampler2D normals;
 uniform sampler2D shadows;
 uniform sampler2D ao;
 uniform sampler2D thick;
 uniform sampler2D reflections;
 uniform isampler2D ids;

#include "constants.inc"

 layout(std430, binding = 4) buffer texbuffer {
	sampler2D textures[];
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

layout(std430, binding = 8) buffer mapbuffer {
	sampler2D normalMaps[];
};

layout(std430, binding = 9) buffer roughbuffer {
	sampler2D roughnessMaps[];
};

layout(std430, binding = 10) buffer metalbuffer {
	sampler2D metalMaps[];
};

#include "helper.inc"
#include "lighting.inc"
                                               
 void main()                                                                         
 {  
	vec3 eye = gEyeWorldPos.xyz;
	vec3 rayDirection =normalize(WorldPos - eye );

	float dist =  texture2D(positions, TexCoord0).x;  
	vec4 normal_enc =textureLod(normals, TexCoord0,0);                                                                             	
	vec3 normal = decodeNormal(normal_enc.xy);
	vec3 reflection = textureLod(reflections, TexCoord0,0).xyz;
	int iid = textureLod(ids, TexCoord0,0).x; 
	float id = float(iid);
	vec3 pos = eye+rayDirection*dist;

	vec3 mSun=normalize(pos-lightPos);

	vec3 col= vec3(0.5);
	//normal*=0.5;
	//normal+=0.5;
	//FragColor = vec4(normal_id_thickness.w,normal_id_thickness.w,normal_id_thickness.w,1);
	float ndist = dist/100.0;
	//FragColor = vec4(ndist,ndist,ndist,1);
	float shadow = texture2D(shadows, TexCoord0.xy).r;
	float occlusion = texture2D(ao, TexCoord0.xy).r;
	//float thickness = texture2D(sss, TexCoord0.xy).r;
	float thickness = textureLod(thick, TexCoord0,0).x;
	//FragColor= vec4(occlusion,occlusion,occlusion,1);
	//FragColor= vec4(shadow,shadow,shadow,1);
	//FragColor= vec4(thickness,thickness,thickness,1);
	//FragColor = vec4(ndist,ndist,ndist,1);
	//FragColor = vec4(normal.x,normal.y,normal.z,1);

	//FragColor = vec4(1,0,0,1);
	//FragColor = vec4(id,id,id,1);
	//FragColor= vec4(reflection,1);
	//return;

	
	if(id == SKY)
	{
		FragColor = vec4(0.01,0.01,0.01,1);
		return;
	}
	else if(id == SUN)
	{
	float angle =dot(mSun,normalize(eye-lightPos));
		FragColor = mix(vec4(1.0, 0.751, 0.673,1.0),vec4(1.0,1.0,1.0,1.0),angle*angle*angle);
		return;
	}

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

	if(ix < 0) ix+=OFFSET;
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

	if(varParams.x==1)
	col=shading(col,pos,normal, thickness, rayDirection , mSun, shadow,occlusion,max(int(id),0));
	else if(id == 4 && varParams.y==1)
	{
		col=reflection;
	}
	else
	{
		col=shadingPBR(col,pos,normal, thickness, rayDirection , mSun, shadow,occlusion,max(int(id),0),roughness,metal,reflection);
	    col = col / (col + vec3(1.0));
	}

	FragColor= vec4(col,1);                        
 }