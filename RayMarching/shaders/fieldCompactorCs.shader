// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=8,local_size_y=8,local_size_z=1) in;

 #include "constants.inc"
 #include "cullingHelper.inc"

 struct Bound {
 vec4 sphere;
 vec4 forward;
 vec4 up;
 vec4 right;
 };

layout(std140, binding = 10) uniform indexbuffer {
	Bound bounds[MAX_OBJECT_COUNT];
};

layout(std430, binding = 11) buffer fieldbuffer {
	int ids[];
};

layout(std430, binding = 13) buffer aofieldbuffer {
	int ids2[];
};

layout(std140, binding = 14) uniform frustumbuffer {
	vec4 farTL;
	vec4 fTR;
	vec4 fTC;
	vec4 nearTL;
	vec4 nTR;
	vec4 nTC;
};

uniform vec3 cone_cos_sin_tan;

vec4 getPlane(vec3 a, vec3 b, vec3 c)
{
	vec3 ab = b-a;
	vec3 ac = c-a;
	vec3 normal = normalize(cross(ab,ac));
	float dist = dot(-normal,a);
	return vec4(normal,dist);
}

void main()                                                                         
 {
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	vec2 recDim = 1.0/ renderDimensions;
	
	vec2 uvTL = (vec2(gl_GlobalInvocationID.xy)+vec2(0,1)) * recDim;
	vec2 uvBR = uvTL+vec2(1,-1)*recDim;
	vec2 uv = uvTL+vec2(0.5,-0.5)*recDim;
	//flip UV origin from BL to TL
	uvTL.y = 1-uvTL.y;
	uvBR.y = 1-uvBR.y;
	//uv=uvTL+recDim;

	int baseIndex=(storePos.x+(storePos.y*int(gl_NumWorkGroups.x*gl_WorkGroupSize.x)))*MAX_OBJECT_COUNT;

	vec4 tpos =matVP * vec4(uv.x*2.0-1.0,uv.y*2.0-1.0,0, 1.0);
	vec3 WorldPos = tpos.xyz/tpos.w;

	vec3 eye = gEyeWorldPos.xyz;
	vec3 rayDirection =normalize(WorldPos - eye );

	//vec3 camTarget = vec3(farTL.w,fTR.w,fTC.w)*-1.0;
	vec3 camTarget = vec3(0,0,1);
	vec4 planes[6];

	vec3 tileNTL = nearTL.xyz+nTR.xyz*uvTL.x+nTC.xyz*uvTL.y;
	vec3 tileFTL = farTL.xyz+fTR.xyz*uvTL.x+fTC.xyz*uvTL.y;
	planes[0] = vec4(camTarget,dot(camTarget,tileNTL));
	planes[1] = vec4(-camTarget,dot(-camTarget,tileFTL));

	vec3 tileNTR = nearTL.xyz+nTR.xyz*uvBR.x+nTC.xyz*uvTL.y;
	vec3 tileNBL = nearTL.xyz+nTR.xyz*uvTL.x+nTC.xyz*uvBR.y;
 	planes[2] = getPlane(tileNTL,tileNBL,tileFTL);
	planes[3] = getPlane(tileNTL,tileFTL,tileNTR);

	vec3 tileNBR = tileNBL+(tileNTR-tileNTL);
	vec3 tileFBL = farTL.xyz+fTR.xyz*uvTL.x+fTC.xyz*uvBR.y;
	planes[4] = getPlane(tileNBL,tileNBR,tileFBL);
	vec3 tileFTR =farTL.xyz+fTR.xyz*uvBR.x+fTC.xyz*uvTL.y;
	planes[5] = getPlane(tileNTR,tileFTR,tileNBR);

	int currentIndex=1;
	int aoIndex=1;
	for(int i = 1; i < objectLength ; ++i)
	{
	vec4 newSphere = bounds[i].sphere;
	//newSphere.w*=1.2;
		if(intersects(newSphere,eye,rayDirection))
		//if(intersectsCone(bounds[i].sphere,eye,rayDirection,cone_cos_sin_tan.xy))
		{
		//if(intersectsFrustumSphere(planes,bounds[i].sphere))
			if(intersectsRayOBB(eye,rayDirection,bounds[i].sphere.xyz,bounds[i].forward,bounds[i].up, bounds[i].right))
			//if(intersectsFrustumBox(planes,bounds[i].sphere.xyz,bounds[i].forward.xyz,bounds[i].up.xyz, bounds[i].right.xyz))
			{
				ids[baseIndex+currentIndex]=i;
				++currentIndex;
			}
		}
	}

	ids[baseIndex]= currentIndex-1;
 }