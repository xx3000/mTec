// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)

layout(std140, binding = 1) uniform params {
	mat4 matVP; 
	vec4 gEyeWorldPos;
	vec4 varParams;
	vec3 lightPos;
	int objectLength;
};

uniform vec2 renderDimensions;

const float PLANE = -3,
			 OBJ = 1,
			 COLS = -1,
             SKY = -4,
             WALLS = -2,
			 SUN = -5;

const float MAX = 100.0;
const int OFFSET = 4;
float PRE = 0.01;
const float MAX_DIST = 350.0;
const bool RENDER_2D = false;

const int MAX_OBJECT_COUNT = 128;

const float PI = 3.141592653;

struct SDF {
	vec4 resolution;
	vec4 color;
	vec4 fillColor;
	vec4 lightParams;
	int fieldId;
	float texSize;
	int transId;
	int texId;
};

// normal packing/ unpacking with octahedron encoding
// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/

vec2 signNotZero(vec2 v) {
return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}
vec2 OctWrap( vec2 v )
{
    return ( 1.0 - abs( v.yx ) ) * signNotZero(v.xy);
}

vec4 encodeNormal (vec3 n)
{
    n /= ( abs( n.x ) + abs( n.y ) + abs( n.z ) );
    n.xy = n.z >= 0.0 ? n.xy : OctWrap( n.xy );
    n.xy = n.xy * 0.5 + 0.5;
    return vec4(n.xy,0,0);
}

vec3 decodeNormal (vec2 encN)
{
    encN = encN * 2.0 - 1.0;
    vec3 n;
    n.z = 1.0 - abs( encN.x ) - abs( encN.y );
    n.xy = n.z >= 0.0 ? encN.xy : OctWrap( encN.xy );
    n = normalize( n );
    return n;
}