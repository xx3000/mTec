// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)


vec3 triplanarMapping(sampler2D tex,vec3 normal, vec3 p,float zoom)
{
	vec3 blending = abs(normal);
	blending = normalize(blending); // Force weights to sum to 1.0
	float b = (blending.x + blending.y + blending.z);
	blending /= vec3(b, b, b);
	vec4 xaxis = texture(tex, p.zy*zoom);
	vec4 yaxis = texture(tex, p.xz*zoom);
	vec4 zaxis = texture(tex, p.xy*zoom);
	return (xaxis * blending.x + yaxis * blending.y + zaxis * blending.z).xyz;
}

vec3 normalMapping(sampler2D tex,vec3 normal, vec3 p,float zoom)
{
float weight = 0.5;
vec2 lookup = vec2(weight,weight);

vec2 xaxis = texture(tex, p.zy*zoom).xy-weight;
vec2 yaxis = texture(tex, p.xz*zoom).xy-weight;
vec2 zaxis = texture(tex, p.xy*zoom).xy-weight;


vec3 bumpNormal1 = vec3(0,xaxis.x,xaxis.y);
vec3 bumpNormal2 = vec3(yaxis.y,0,yaxis.x);
vec3 bumpNormal3 = vec3(zaxis.x,zaxis.y,0);

vec3 blending = abs(normal);
	blending = normalize(blending); // Force weights to sum to 1.0
	float b = (blending.x + blending.y + blending.z);
	blending /= vec3(b, b, b);
	vec3 blended_bump = bumpNormal1 * blending.x +  
                   bumpNormal2 * blending.y +  
                   bumpNormal3 * blending.z;

return normalize(normal+blended_bump);
}


