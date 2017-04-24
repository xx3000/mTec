// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)

struct TraceResult
{
	vec3 hit;
	float distance;
	float steps;
	float sdf;
	bool aborted;
};

TraceResult fastMarch(vec3 p, vec3 d, TraceResult res, float push, float max, float threshold)
{
	vec3 P = p + d*push;
	float T = res.distance + push;
	float steps;

	for (float i = 0.0; i<max; i++)
	{
		float S = combinedSDF(P);
		float radius = abs(S);

		radius = radius / T;
		T += S;
		P = p + d*T;
		steps = i;
		if ((T>(MAX_DIST)) || (radius < threshold)) {
			break;
		}
	}
	res.aborted = steps + 1 >= max;
	res.steps += steps;
	res.hit = P;
	res.distance = T;
	return res;
}

TraceResult fastMarch(vec3 p, vec3 d, TraceResult res, float push, float max, float threshold, int cullIndex)
{
	vec3 P = p + d*push;
	float T = res.distance + push;
	float steps;

	for (float i = 0.0; i<max; i++)
	{
		float S = GetSdfDist(P,cullIndex);
		float radius = abs(S);

		radius = radius / T;
		T += S;
		P += d*S;
		steps = i;
		if ((T>(MAX_DIST)) || (radius < threshold)) {
			break;
		}
	}
	res.aborted = steps + 1 >= max;
	res.steps += steps;
	res.hit = P;
	res.distance = T;
	return res;
}


TraceResult worldMarch(vec3 p, vec3 d)
{
	TraceResult tr;
	tr.distance=0;
	tr.aborted = false;
	tr.steps = 0;
	float threshold = 0.0001;

	float push =1.0;
	float max =250.0;
	vec3 P = p + d*push;
	float T = tr.distance + push;
	float steps;

	for (float i = 0.0; i<max; i++)
	{
		float S = pollWorldSdf(P)+0.1;
		//if( S < 0.2)
		//S= combinedSDFNoSun(P);
		float radius = abs(S);

		radius = radius / T;
		T += S;
		P += d*S;
		steps = i;
		if ((T>(MAX_DIST)) || (radius < threshold)) {
			break;
		}
	}
	tr.aborted = steps + 1 >= max;
	tr.steps += steps;
	tr.hit = P;
	tr.distance = T;

	if (tr.distance >= MAX_DIST)
		tr.sdf = SKY;
	else
		tr.sdf = pollSDFIdNC(tr.hit);

	return tr;
}


TraceResult combinedMarch(vec3 p, vec3 d, int cullIndex)
{
	TraceResult tr;
	tr.distance=0;
	tr.aborted = false;
	tr.steps = 0;
	//float threshold = gEyeWorldPos.w*5.0;
	float threshold = gEyeWorldPos.w;

	tr = fastMarch(p, d, tr, 0.01, 250, threshold,cullIndex);

	if (tr.distance >= MAX_DIST)
		tr.sdf = SKY;
	else
		tr.sdf = pollSDFId(tr.hit,cullIndex);


	if (tr.aborted)
	{
		float skyd = combinedSDF(tr.hit + d*250.0);
		if (skyd > 0)
			tr.sdf = SKY;
	}

	return tr;
}

float ThicknessTrace(vec3 p, vec3 d, float stepSize, float maxThickness, float maxSteps,int id)
{
	float S;
	vec3 P = p + d*stepSize;

	float thickness = 0.0;

	float lastS = -1;
	for (float i = 0.0; i<maxSteps; i++)
	{
		S = sdfSpec(P,id).x;
		P += d*stepSize;

		if (S < 0)
		{
			//if(lastS > 0)
			thickness += min(abs(S), stepSize);
			//else
			//	thickness += stepSize;
		}

		lastS = S;

		if (thickness > maxThickness)
			break;

	}

	return 1 - min(1.0, thickness / maxThickness);
}