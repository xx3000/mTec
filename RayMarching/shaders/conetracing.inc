// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)

//SimpleConeTrace(res.hit,dir,20.0,0.02,30.0,0.15,0.0,8.0)
float SimpleConeTrace(vec3 p, vec3 d, vec3 d2, float distance, float rIncrease, float maxSteps, float cutoff, float multiplier, float minStepSize,int cullIndex)
{
	bool aborted = true;
	float lightDist = length(p - lightPos);
	float closestPass = 1.0;
	//float push = rIncrease*pow(lightDist,1.5)*0.25;
	//float push = rIncrease * max(1.0,lightDist-10.0);
	float push = rIncrease;
	float radius = push;
	float maxPush = 0.8;
	vec3 P = p;
	//maxSteps=155;
	maxSteps+=16.0;
	//float dist = distance;
	float maxDist = 50.0;
	float minS = 0.0;

	P += d*push;
	int steps = 0;
	float dist = push;

	int tries = 0;
	{

		for (int i = 0; i<maxSteps; i++)
		{
			float S = GetSdfDistNoSun(P,radius,cullIndex);
			//float S = combinedSDFNoSun(P);
			closestPass = min(closestPass, (multiplier*S / radius));
			//radius+=S;
			// radius+= max( S, 0.02);
			radius += clamp(S, 0.02, 0.1);
			dist += max(minS, S);
			P += d*max(minS, S);
			steps = i;
			if (dist >= lightDist || dist>maxDist ) {
				break;
			}

			if(S < cutoff)
			{
				closestPass =0.0;
				break;
			}
		}
	}

	return clamp(closestPass, 0, 1);
}

float AoConeTrace(vec3 p, vec3 d, float distance, float rIncrease, float maxSteps, float cutoff, float multiplier)
{
	float closestPass = 1.0;
	float radius = rIncrease;
	vec3 P = p;
	float maxDist = distance;

	P += d*rIncrease;
	float dist = rIncrease;

	for (int i = 0; i<maxSteps; i++)
	{
		float S = combinedSDFNoSun(P);
		closestPass = min(closestPass, (multiplier*S / radius));
		//radius+=S;
		// radius+= max( S, 0.02);
		radius += clamp(S, 0.02, 0.1);
		dist += S;
		P += d*S;
		if (dist >= maxDist || S < cutoff ) {
			break;
		}
	}
	
	return clamp(closestPass, 0, 1);
}