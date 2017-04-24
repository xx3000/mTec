/*
Copyright (c) <2015> <Playdead> L.Pedersen, MIT Licence

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

layout (local_size_x=8,local_size_y=8,local_size_z=1) in;

restrict writeonly uniform image2D velocityBuffer;
uniform sampler2D positions;
uniform mat4 historyVP;

#include "constants.inc"

void main()                                                                         
 {
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = (vec2(gl_GlobalInvocationID.xy)+vec2(0.5,0.5)) / renderDimensions;
	
	vec4 tpos =matVP * vec4(uv.x*2.0-1.0,uv.y*2.0-1.0,0, 1.0);
	vec3 WorldPos = tpos.xyz/tpos.w;

	float dist =  texture2D(positions, uv).x;  
	vec3 eye = gEyeWorldPos.xyz;
	vec3 rayDirection =normalize(WorldPos - eye );
	vec3 pos = eye+rayDirection*dist;


	// reproject into previous frame
	vec4 rp_cs_pos = historyVP * vec4(pos,1.0);
	vec2 rp_ss_ndc = rp_cs_pos.xy / rp_cs_pos.w;
	vec2 rp_ss_txc = 0.5 * rp_ss_ndc + 0.5;

	// estimate velocity
	vec2 ss_vel = uv - rp_ss_txc;

	imageStore(velocityBuffer, storePos, vec4(ss_vel,0,0));
	
	//imageStore(velocityBuffer, storePos, vec4(1,1,1,1));
	
 }