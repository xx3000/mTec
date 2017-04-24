// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)


#version 430  
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require     
#extension GL_ARB_conservative_depth : require                                                               

 in vec2 TexCoord0;
 in vec3 WorldPos;                                                                  
 

layout (depth_greater) out float gl_FragDepth;
layout(location = 0) out vec4 FragColor;                                                            

uniform int index;
uniform sampler3D field;
uniform mat4 invWorld;

sampler3D fields[1];
mat4 transforms[1];
int ids[1];

#include "constants.inc"




layout(std140, binding = 5) uniform indexbuffer {
	SDF objects[MAX_OBJECT_COUNT];
};

#include "distanceFunctions.inc"

float lengthSqr(vec3 a)
{
	return dot(a,a);
}

float sdfSpec(vec3 p, sampler3D sdf)
{
	float sdfval=textureLod(sdf,p,0).r;	
    return sdfval;
}

float nonlinearDepth(float depth)
{
float zNear=1.0;
float zFar=100.0;

//float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
	float A = (zFar)/(zFar-zNear);
    float B = (zFar*zNear)/(zFar-zNear);
   return 0.5*((A*depth + B) / depth)+0.5 ;
 // return -(((2.0*zNear*zFar)/depth)-zFar-zNear)/(zFar-zNear);
}

struct TraceResult
{
	vec3 hit;
	float distance;
	float steps;
	float sdf;
	bool aborted;
};

vec3 calcNormal( vec3 pos, int id,vec4 scale)
{
	vec3  eps = vec3( 0.1, 0.0,0.0 );
	vec3 offset = vec3(0.5);
	//pos*=scale.xyz;
	return normalize( vec3( sdfSpec(offset+((pos-eps.xyz)*scale.xyz),field).x - sdfSpec(offset+((pos+eps.xyz)*scale.xyz),field).x,
							sdfSpec(offset+((pos-eps.zxy)*scale.xyz),field).x - sdfSpec(offset+((pos+eps.zxy)*scale.xyz),field).x,
							sdfSpec(offset+((pos-eps.zyx)*scale.xyz),field).x - sdfSpec(offset+((pos+eps.zyx)*scale.xyz),field).x ) );
 
}

TraceResult blockMarch(vec3 p, vec3 d, sampler3D sdf,vec4 scale)
{
	TraceResult tr;
	tr.distance=0;
	tr.aborted = false;
	tr.steps = 0;
	//float threshold = gEyeWorldPos.w*5.0;
	float threshold = gEyeWorldPos.w*objects[index].color.w;

	float max =150.0;
	float push = 0.01;

	vec3 P = p + d*push;
	float T = tr.distance + push;
	float steps;

	for (float i = 0.0; i<max; i++)
	{
		vec3 scaledP = 	P*scale.xyz;
		scaledP+= vec3(0.5,0.5,0.5);
		vec3 clamped = vec3(clamp(scaledP,0,1)); 
		if(lengthSqr(scaledP-clamped) > 0.001)
		{
			steps = max;
			break;
		}

		float S = sdfSpec(scaledP,sdf);
		S*=scale.w;
		float radius = abs(S);

		radius = radius / T;
		T += S;
		P += d*S;
		steps = i;
		if ( (radius < threshold) ) {
			break;
		}
	}
	tr.aborted = steps + 1 >= max;
	tr.steps += steps;
	tr.distance = T;
	tr.hit = P;
	return tr;
}
	                   
 void main()                                                                         
 {  
	vec3 camPos = gEyeWorldPos.xyz;

	vec3 dir = normalize(WorldPos-camPos);

	mat4 tranf =invWorld;
		float time = tranf[0][3];
		tranf[0][3]=0;
	vec3 pos= (tranf*vec4(WorldPos,1.0)).xyz;
	
	//vec3 pos=WorldPos;
	vec4 scale =vec4(objects[index].color.w*objects[index].resolution.xyz,objects[index].resolution.w);
	// get intersection distance
	TraceResult res;
    res=blockMarch(pos,dir,field,scale);
	res.distance = length(WorldPos-camPos)+length(pos-res.hit);

	res.sdf=index;
	float dist = (res.distance)/100.0f;
	
	gl_FragDepth= res.aborted ? 1.0:dist;
	if(res.aborted)
	{
		//gl_FragDepth=1.0;
		FragColor= vec4(0,0,0,0);
		return;
	}
	float depth=nonlinearDepth(res.distance-1.0);
	
	FragColor= vec4(res.distance,calcNormal(res.hit,index,scale));                     
 }