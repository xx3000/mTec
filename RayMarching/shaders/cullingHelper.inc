// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)


float lengthSqr(vec3 a)
{
	return dot(a,a);
}


bool intersectsSphere(vec4 sphere1, vec4 sphere2)
{
	float sqrdist = sphere1.w+ sphere2.w;
	return lengthSqr(sphere1.xyz - sphere2.xyz) < sqrdist*sqrdist;
}

		// sphere ray intersection
bool intersects(vec4 sphere,vec3 origin, vec3 dir)
{
//sphere.xyz*=-1.0;
	vec3 os = sphere.xyz-origin;
	float dist = dot(os,dir);
	if(dist < -sphere.w)
	return false;

	vec3 projPoint = origin+dir*dist;
	vec3 ps = sphere.xyz-projPoint;

	return lengthSqr(ps) < (sphere.w*sphere.w);
}

bool intersectsNR(vec4 sphere,vec3 origin, vec3 dir, vec3 target)
{
//sphere.xyz*=-1.0;
	vec3 os = sphere.xyz-origin;
	float dist = dot(os,dir);

	vec3 projPoint = origin+dir*dist;
	vec3 ps = sphere.xyz-projPoint;
	float projDist = lengthSqr(origin-projPoint);
	float len2 = lengthSqr(ps);
	return projDist < lengthSqr(target) && len2 < (sphere.w*sphere.w);
}

//Sphere cone intersection
// https://www.geometrictools.com/Documentation/IntersectionSphereCone.pdf

bool intersectsCone(vec4 sphere,vec3 origin, vec3 dir, vec2 cone_cos_sin)
{
	vec3 U = origin - (sphere.w*cone_cos_sin.y)*dir;
	vec3 D = sphere.xyz - U;
	float dsqr =lengthSqr(D);
	float e = dot(dir,D);
	vec3 os = sphere.xyz-origin;
	float dist = dot(os,dir);
	return dist > -sphere.w && e > 0 && e*e >= dsqr*(cone_cos_sin.x);
}

int checkPoint(vec4 plane, vec3 point)
{
	return (dot( plane, vec4(point, 1.0f) ) < 0.0 ) ? 1:0;
}

bool checkSphere(vec4 plane, vec3 point, float radius)
{
	return (dot( plane, vec4(point, 1.0f) ) + radius < 0.0 );
}

// ray obb intersection RTR3
bool intersectsRayOBB (vec3 origin, vec3 dir,vec3 center,vec4 fwd,vec4 top, vec4 right)
{
float tmin = -999999999;
float tmax = 999999999;

vec3 p =center-origin;

vec3 dirs[3];
dirs[0]= right.xyz;
dirs[1]= top.xyz;
dirs[2]= fwd.xyz;
float lengths[3];
lengths[0]= right.w;
lengths[1]= top.w;
lengths[2]= fwd.w;
for(int i =0 ; i < 3 ; ++i)
{
	float e = dot(dirs[i],p);
	float f = dot(dirs[i],dir);
	if(abs(f) > 0.0)
	{
		float invf=1.0/f;
		float t1 = (e+lengths[i])*invf;
		float t2 = (e-lengths[i])*invf;
		if(t1 > t2)
		{
			float swap = t2;
			t2=t1;
			t1=swap;
		}
		if(t1 > tmin) tmin=t1;
		if(t2 < tmax) tmax=t2;
		if(tmin > tmax) return false;
		if(tmax < 0) return false;
	}
	else if((-e-lengths[i]) > 0 || (-e+lengths[i]) < 0) return false;
}

return true;
}


// frustum/OBB intersection Moller, Real-Time Rendering 3ed p777
bool intersectsFrustumBox(vec4 planes[6],vec3 center,vec3 fwd,vec3 top, vec3 right)
{
for( int i=0; i<6; ++i )
    {
        int isct = 0;
		vec3 point;
		vec4 plane=planes[i];

		//point= center;
       // isct += checkPoint(plane,point);
		
		point= center+fwd+top+right;
        isct += checkPoint(plane,point);
		point= center-fwd-top+right;
        isct += checkPoint(plane,point);
		point= center+fwd-top+right;
        isct += checkPoint(plane,point);
		point= center-fwd+top+right;
        isct += checkPoint(plane,point);

		point= center+fwd+top-right;
        isct += checkPoint(plane,point);
		point= center+fwd-top-right;
        isct += checkPoint(plane,point);
		point= center-fwd-top-right;
        isct += checkPoint(plane,point);
		point= center-fwd+top-right;
        isct += checkPoint(plane,point);
		
        if( isct==8 ) return false;
    }

    return true;

}

bool intersectsFrustumSphere(vec4 planes[6],vec4 sphere)
{
for( int i=0; i<6; ++i )
    {
        int isct = 0;
		vec3 point;
		vec4 plane=planes[i];
        if (checkSphere(plane,sphere.xyz,sphere.w))
		return false;

    }

    return true;

}
