// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)


 #version 330								  
 
layout (location = 0) in vec3 Position;   
layout (location = 1) in vec2 TexCoord;   

uniform mat4 gVP;
uniform mat4 gWorld;										
											 
out vec2 TexCoord0;	
out vec3 WorldPos;						
											 
void main()									
{											
				
gl_Position =  vec4(Position.xy,10000.0, 1.0);
vec4 tpos = gVP * vec4(Position, 1.0);
WorldPos= tpos.xyz / tpos.w;                                   
TexCoord0 = TexCoord;									
				
}  

