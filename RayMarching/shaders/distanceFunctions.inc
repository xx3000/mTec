// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)

uniform sampler3D world;
uniform isampler3D worldIds;

vec2 cmin(vec2 a, vec2 b)
{
if(a.x < b.x ) return a;
return b;
}

float opU( float d1, float d2 )
{
    return min(d1,d2);
}

vec2 opU( vec2 d1, vec2 d2 )
{
    return d1.x < d2.x  ? d1 : d2;
}

float opS(float d1, float d2)
{
    return max(-d1,d2);
}

float smin( float a, float b, float k )
{
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

vec2 roundBox( vec3 p)
{
//vec3 center=vec3(0,5.2,0);
//return vec2(length(p-center)-0.5,BOX);
  return vec2(length(max(abs((p)+vec3(1,-1.0,6))-vec3(2,2,2),0.0))-0.0,WALLS);
}

float boxPrimitive(vec3 p, vec3 c, vec3 ex)
{
    vec3 d = abs(p+c) - ex;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
    //return length(max(abs(p+c)-ex,0.0));
}

float sdTriPrism( vec3 p, vec2 h ,vec3 c)
{
    p-=c;
    vec3 q = abs(p);
    return max(q.z-h.y,max(q.x*0.266025+p.y*0.5,-p.y)-h.x*0.5);
}

vec2 box(vec3 p)
{
//hall
float dist = boxPrimitive(p,vec3(0,0,0),vec3(28,34,44));
float dist2 = boxPrimitive(p,vec3(0,0,0),vec3(25,35,41));
dist= opS(dist2,dist);  

dist = opU(dist,sdTriPrism(p,vec2(10,44),vec3(0,36.5,0)));
dist = opS(sdTriPrism(p,vec2(9,43),vec3(0,33.5,0)),dist);
dist = opU(dist,boxPrimitive(p,vec3(0,-20,0),vec3(26,1,42)));
dist = opS(boxPrimitive(p,vec3(0,-20,0),vec3(7,2,23)),dist);

return vec2(dist,WALLS);

}

vec2 cols(vec3 p)
{
//return vec2(999,COLS);
// pillars
float c = 16.5;
vec3 q = mod(p,c)-0.5*c;
q.y=p.y;
float dist = boxPrimitive(q,vec3(0,0,0),vec3(1,42,1));
return vec2(dist,COLS);
}

vec2 room(vec3 p)
{
return vec2(opU(box(p).x,cols(p).x),COLS);
}

float sphere( vec3 p,vec3 center,float radius)
{
return length(p-center)-radius;
  //return vec2(length(max(abs((p)+vec3(16,-1.0,6))-vec3(2,2,2),0.0))-1.0,WALLS);
}

vec2 sun ( vec3 p)
{
return vec2(length(p-(lightPos))-0.5,SUN);
//return vec2(99999,SUN);
}



float sphere ( vec3 p)
{
vec2 t = vec2(0.7,0.3);
vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
//return length(p)-0.6;
}

vec2 plane(vec3 p)
{
return vec2(p.y,PLANE);
}

float sdOctahedron(vec3 p, vec3 center, float radius) 
{
   float sp =sphere(p,vec3(0,0.095,0),0.23);
    return sp;	
}

float pollField(vec3 p,int id,float time)
{
	vec3 transP=p;
	transP*= objects[id].resolution.xyz;  //adjust to correct scale
	transP+= vec3(0.5,0.5,0.5);
	// distance clamp
	vec3 clamped = vec3(clamp(transP.x,0,1),clamp(transP.y,0,1),clamp(transP.z,0,1)); 
	float sdfval=mix(texture(fields[objects[id].fieldId],clamped).r,sdOctahedron(p,vec3(0.0),1.3),time);


	float len = length(transP-clamped);

	sdfval+=len*0.8;
    return sdfval;
}

vec2 sdf(vec3 p)
{
	//return vec2(999999,float(1.0));
	
	float sdfval=9999999;
	int id = -1;
	for(int i =1; i < objectLength ; ++i)
	{
		vec3 transP=p;
		mat4 tranf =transforms[objects[i].transId];
		float time = tranf[0][3];
		tranf[0][3]=0;
		transP= (tranf*vec4(transP,1.0)).xyz;
		transP*=objects[i].color.w;

		float tmpVal = pollField(transP,i,time);
		tmpVal*=objects[i].resolution.w;
		//float sdfval = pollField(transP,i);
	   // float sp =sphere(transP,vec3(-0.05,0.1,0));
	   // sdfval = max(-sp,sdfval);
	   if(tmpVal < sdfval)
	   {
			sdfval= tmpVal;
			id= i;
	   }

   }
    return vec2(sdfval,float(id));
}

vec2 culledSdf(vec3 p, int cullIndex)
{
//return sdf(p);
	float sdfval=9999999;
	int id = -1;
	int length =ids[cullIndex]+1;
	for(int i =1 ; i < length ; ++i)
	{
		int index = ids[cullIndex+i];
		vec3 transP=p;
		mat4 tranf =transforms[objects[index].transId];
		float time = tranf[0][3];
		tranf[0][3]=0;
		transP= (tranf*vec4(transP,1.0)).xyz;
		transP*=objects[index].color.w;
		float tmpVal = pollField(transP,index,time);
		tmpVal*=objects[index].resolution.w;
	   if(tmpVal < sdfval)
	   {
			sdfval= tmpVal;
			id= index;
	   }
   }
    return vec2(sdfval,float(id));
}

float GetSDFCulled(vec3 p, int cullIndex)
{

return cmin(cols(p),cmin(cmin(culledSdf(p,cullIndex),plane(p)),cmin(box(p),sun(p)))).y;
}

float GetSDF(vec3 p)
{
return cmin(cols(p),cmin(cmin(sdf(p),plane(p)),cmin(box(p),sun(p)))).y;
}

vec2 pollWorldSdf(vec3 p)
{
//return vec2(-1.0,0);
	vec3 transP=p;
	transP= (transforms[objects[0].transId]*vec4(transP,1.0)).xyz;
	transP*= objects[0].resolution.xyz; 
	transP+= vec3(0.5,0.5,0.5);
	// distance clamp
	vec3 clamped = vec3(clamp(transP.x,0,1),clamp(transP.y,0,1),clamp(transP.z,0,1)); 
	float sdfval=textureLod(world,clamped,0).r;
    return vec2(sdfval,1.0);
}

int pollWorldIds(vec3 p)
{
   // return int(GetSDF(p));
//return vec2(-1.0,0);

	vec3 transP=p;
	transP= (transforms[objects[0].transId]*vec4(transP,1.0)).xyz;
	transP*= objects[0].resolution.xyz; 
	transP+= vec3(0.5,0.5,0.5);
	// distance clamp
	vec3 clamped = vec3(clamp(transP.x,0,1),clamp(transP.y,0,1),clamp(transP.z,0,1)); 
	int sdfval=textureLod(worldIds,clamped,0).r;
    return sdfval;
}

float pollSDFId(vec3 p,int cullIndex)
{
float sdfval =float(pollWorldIds(p));
	if(sdfval ==  0)
	sdfval = GetSDFCulled(p,cullIndex);
	return sdfval;
}

float pollSDFIdNC(vec3 p)
{
float sdfval =float(pollWorldIds(p));
	if(sdfval ==  0)
	sdfval = GetSDF(p);
	return sdfval;
}

vec2 sdfSpec(vec3 p, int id)
{
	vec3 transP=p;
	mat4 tranf =transforms[objects[id].transId];
		float time = tranf[0][3];
		tranf[0][3]=0;
	transP= (tranf*vec4(transP,1.0)).xyz;
	transP*=objects[id].color.w;

	float sdfval = pollField(transP,id,time);
	sdfval*=objects[id].resolution.w;
    return vec2(sdfval,float(id));
}

float environmentNoSunSDF(vec3 p)
{
return opU(cols(p).x,opU(plane(p).x,box(p).x));
}

float environmentSDF(vec3 p)
{
return opU(environmentNoSunSDF(p),sun(p).x);
}

vec2 environmentNoSunSDFData(vec3 p)
{
return opU(cols(p),opU(plane(p),box(p)));
}

vec2 environmentSDFData(vec3 p)
{
return opU(environmentNoSunSDFData(p),sun(p));
}

float combinedSDFNoSun(vec3 p)
{
	return opU(environmentNoSunSDF(p),sdf(p).x);
}

float combinedSDFNoSunCull(vec3 p,int cullIndex)
{
	return opU(environmentNoSunSDF(p),culledSdf(p,cullIndex).x);
}

float aoSdfDist(vec3 p,float dist,int cullIndex)
{
float world = -999.0;
	
	if(dist > 0.75)
	{
		world = pollWorldSdf(p).x;	
	}
	else
	{
		int id = pollWorldIds(p);
		if(id == 0)
		{
		//world =combinedSDF(p);
		world =combinedSDFNoSunCull(p,cullIndex);
		}
		else if( id < 0)
		{
		world = environmentNoSunSDF(p);
		}
		else
		{
		world = sdfSpec(p,id);
		}
		//world = combinedSDFNoSunCull(p,cullIndex);
		//world = combinedSDFNoSun(p);
	}
	return world;
	//return opU(cols(p),opU(opU(culledSdf(p,cullIndex).x,plane(p).x),box(p).x));
}


vec2 combinedSDFData(vec3 p)
{
	return opU(sdf(p),environmentSDFData(p));
}

float combinedSDF(vec3 p)
{
	return opU(sdf(p).x,environmentSDF(p));
}

float combinedSDFcull(vec3 p,int cullIndex)
{
//return pollWorldSdf(p).x;
return opU(culledSdf(p,cullIndex).x,environmentSDF(p));
}

float GetSdfDist(vec3 p, int cullIndex)
{
//return sdfSpec(p,cullIndex);
float world = -999.0;
	world = pollWorldSdf(p).x;
	//return world;
if(world < 0.5)
{

//world =combinedSDFcull(p,cullIndex);
//return world;
	int id = pollWorldIds(p);
	if(id == 0)
	{
	//world =combinedSDF(p);
	world =combinedSDFcull(p,cullIndex);
	}
	else if( id < 0)
	{
	world = environmentSDF(p);
	}
	else
	{
	world = sdfSpec(p,id);
	}

}

return world;
}


float GetSdfDistNoSun(vec3 p, float radius,int cullIndex)
{
	float world = -999.0;
	//world = pollWorldSdf(p).x;
	//if(world < 0.0)
	{
		int id = pollWorldIds(p);
		if(id == 0)
		{
		//world = combinedSDFNoSun(p);	
		world =combinedSDFNoSunCull(p,cullIndex);
		}
		else if( id < 0)
		{
		world = environmentNoSunSDF(p);
		}
		else
		{
		world = sdfSpec(p,id);
		}
		//world = combinedSDFNoSunCull(p,cullIndex);
		//world = combinedSDFNoSun(p);	
	}
	return world;
}


