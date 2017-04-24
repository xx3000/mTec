// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#version 430  
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require     
#extension GL_ARB_conservative_depth : require                                                               

 in vec2 TexCoord0;
 in vec3 WorldPos;                                                                  
 

//layout (depth_greater) out float gl_FragDepth;
layout(location = 0) out vec4 FragColor;        
layout(early_fragment_tests) in;                                                    

uniform int index;

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

vec3 calcNormal( vec3 pos, int id)
{
	vec3  eps = vec3( 0.1, 0.0,0.0 );
	vec3 offset = vec3(0.5);
	//pos*=scale.xyz;
	return normalize( vec3( room(pos-eps.xyz).x - room(pos+eps.xyz).x,
							room(pos-eps.zxy).x - room(pos+eps.zxy).x,
							room(pos-eps.zyx).x - room(pos+eps.zyx).x ) );
}

TraceResult blockMarch(vec3 p, vec3 d)
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
		float S = environmentSDF(P);
		float radius = abs(S);

		radius = radius / T;
		T += S;
		P = p + d*T;
		steps = i;
		if ((T>(100)) || (radius < threshold)) {
			break;
		}
	}

	tr.aborted = steps + 1 >= max;
	tr.steps += steps;
	tr.distance = T;
	tr.hit = P;
	tr.sdf =environmentSDFData(P).y;
	return tr;
}
	                   
 void main()                                                                         
 {  
	vec3 camPos = gEyeWorldPos.xyz;

	vec3 dir = normalize(WorldPos-camPos);

	vec3 pos=WorldPos;
	TraceResult res;
    res=blockMarch(pos,dir);
	res.distance = length(WorldPos-camPos)+length(pos-res.hit);

	//res.sdf=index;
	float dist = (res.distance)/100.0f;
	
	//gl_FragDepth= 1.0;
	if(res.aborted)
	{
		//gl_FragDepth=1.0;
		FragColor= vec4(0,0,0,0);
		return;
	}
	//float depth=nonlinearDepth(res.distance-1.0);
	
	FragColor= vec4(res.distance,calcNormal(camPos+dir*res.distance,index));                     
 }