// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)


//iq AO

float AmbientOcclude(in vec3 pos, in vec3 nor)
{
	float ao = 0.0;
	float sca = 0.5;
	float delta = 0.25;
	for (int i = 0; i<5; i++)
	{
		float hr = (delta + delta *i);
		float dd = combinedSDFNoSun(pos + hr*nor);
		ao += (hr - dd)*sca;
		sca *= 0.5;
	}
	return 1 - clamp(ao, 0.0, 1.0);
}

#define SQRT2 1.51421356

float cao(in vec3 pos, in vec3 nor, float multiplier,int cullIndex)
{
	float ao = 0.8125;
	float sca = 1.0;
	float delta = 0.25;
	for (int i = 0; i<4; i++)
	{
		float hr = (delta + delta *i);
		float dd = aoSdfDist(pos + hr*nor,hr,cullIndex);
		if(dd < 0.0 )
		break;
		dd *= multiplier;
		dd=clamp(dd, 0.0, hr);
		ao -= dd*sca;
		sca *= 0.5;
	}
	ao*= 1.2307692;
	return 1-clamp(ao, 0.0, 1.0);
}



//UE4 AO
float AmbientOcclude(vec3 p, vec3 normal, float distance, float rIncrease, float maxSteps, float cutoff)
{
	const int cones = 9;
	vec3 dirs[cones];
	dirs[0] = normal;
	vec3 up = vec3(0, -1, 0);
	if (abs(dot(normal, up)) >= 0.9)
		up = vec3(1, 0, 0);
	vec3 right = normalize(cross(normal, up));
	vec3 front = normalize(cross(right, normal));
	vec3 fr = (front + right)*0.5;
	vec3 fl = (front - right)*0.5;
	vec3 br = (right - front)*0.5;
	vec3 bl = (-right - front)*0.5;
	dirs[1] = (normal + right)*0.5;
	dirs[2] = (normal - right)*0.5;
	dirs[3] = (normal + front)*0.5;
	dirs[4] = (normal - front)*0.5;
	dirs[5] = (normal + fr)*0.5;
	dirs[6] = (normal + fl)*0.5;
	dirs[7] = (normal + br)*0.5;
	dirs[8] = (normal + bl)*0.5;

	float minOcc = 1.0;
	for (int i = 0; i < cones; ++i)
	{
		minOcc += AoConeTrace(p, dirs[i], distance, rIncrease, maxSteps, cutoff, 1.8);
	}
	minOcc /= cones;
	return minOcc;
}

float AmbientOcclude(vec3 p, vec3 normal, int cullIndex)
{
	const int cones = 5;
	vec3 dirs[cones];
	
	vec3 up = vec3(0, -1, 0);
	if (abs(dot(normal, up)) >= 0.9)
		up = vec3(1, 0, 0);
	vec3 right = normalize(cross(normal, up));
	vec3 front = normalize(cross(right, normal));
	dirs[0] = (normal + right)*0.75;
	dirs[1] = (normal - right)*0.75;
	dirs[2] = (normal + front)*0.75;
	dirs[3] = (normal - front)*0.75;
	dirs[4] = normal;

	float minOcc = 1.0;
	for (int i = 0; i < cones; ++i)
	{
		minOcc += cao(p, dirs[i], 0.9,cullIndex);
		//minOcc= min(minOcc,cao(p,dirs[i],SQRT2));
	}
	//minOcc += cao(p, dirs[0], 1,cullIndex);
	minOcc /= cones;
	//minOcc/=(cones-1);
	return minOcc;
}
