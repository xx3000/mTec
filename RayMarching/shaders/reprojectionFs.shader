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
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_bindless_texture : require

 in vec2 TexCoord0;
 in vec3 WorldPos;                                                                  
 
 #include "noise.inc"

layout(location = 0) out vec4 FragColor;                                                            

 uniform sampler2D frame;
 uniform sampler2D pastHistory;
 uniform sampler2D positions;
 uniform sampler2D velocity;
restrict writeonly uniform image2D newHistory;

layout(std140, binding = 1) uniform params {
	mat4 matVP; 
	mat4 historyVP;
	vec4 renderDimensions;
	vec4 jitter;
	vec4 sinTime;
};

const float FLT_EPS = 0.00000001f;

float Luminance(vec3 c)
{
// https://en.wikipedia.org/wiki/Grayscale
    return dot(c, vec3(0.2126, 0.7152, 0.0722));

}

vec4 sample_color_motion(sampler2D tex, vec2 uv, vec2 ss_vel)
	{
		const vec2 v = 0.5 * ss_vel;
		const int taps = 3;// on either side!

		float srand = PDsrand(uv + sinTime.xx);
		vec2 vtap = v / taps;
		vec2 pos0 = uv + vtap * (0.5 * srand);
		vec4 accu = vec4(0.0);
		float wsum = 0.0;

		for (int i = -taps; i <= taps; i++)
		{
			float w = 1.0f;// box
			accu += w * textureLod(tex, pos0 + i * vtap,0);
			wsum += w;
		}

		return accu / wsum;
	}

vec3 find_closest_fragment(vec2 uv)
	{
		vec2 dd = renderDimensions.zw;
		vec2 du = vec2(dd.x, 0.0);
		vec2 dv = vec2(0.0, dd.y);

		vec3 dtl = vec3(-1, -1, textureLod(positions, uv - dv - du,0).x);
		vec3 dtc = vec3( 0, -1, textureLod(positions, uv - dv,0).x);
		vec3 dtr = vec3( 1, -1, textureLod(positions, uv - dv + du,0).x);
								
		vec3 dml = vec3(-1,  0, textureLod(positions, uv - du,0).x);
		vec3 dmc = vec3( 0,  0, textureLod(positions, uv,0).x);
		vec3 dmr = vec3( 1,  0, textureLod(positions, uv + du,0).x);
								
		vec3 dbl = vec3(-1,  1, textureLod(positions, uv + dv - du,0).x);
		vec3 dbc = vec3( 0,  1, textureLod(positions, uv + dv,0).x);
		vec3 dbr = vec3( 1,  1, textureLod(positions, uv + dv + du,0).x);

		vec3 dmin = dtl;
		if (dmin.z > dtc.z) dmin = dtc;
		if (dmin.z > dtr.z) dmin = dtr;

		if (dmin.z > dml.z) dmin = dml;
		if (dmin.z > dmc.z) dmin = dmc;
		if (dmin.z > dmr.z) dmin = dmr;

		if (dmin.z > dbl.z) dmin = dbl;
		if (dmin.z > dbc.z) dmin = dbc;
		if (dmin.z > dbr.z) dmin = dbr;

		return vec3(uv + dd.xy * dmin.xy, dmin.z);
	}
      
	  vec4 clip_aabb(vec3 aabb_min, vec3 aabb_max, vec4 p, vec4 q)
	{
	//#if USE_OPTIMIZATIONS
		// note: only clips towards aabb center (but fast!)
		vec3 p_clip = 0.5 * (aabb_max + aabb_min);
		vec3 e_clip = 0.5 * (aabb_max - aabb_min);

		vec4 v_clip = q - vec4(p_clip, p.w);
		vec3 v_unit = v_clip.xyz / e_clip;
		vec3 a_unit = abs(v_unit);
		float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

		if (ma_unit > 1.0)
			return vec4(p_clip, p.w) + v_clip / ma_unit;
		else
			return q;// point inside aabb
	}   

	 vec4 temporal_reprojection(vec2 ss_txc, vec2 ss_vel, float vs_dist)
	{
	//if UNJITTER_COLORSAMPLES || UNJITTER_NEIGHBORHOOD
		vec2 jitter0 = jitter.xy * renderDimensions.zw;

		// read texels
	//if UNJITTER_COLORSAMPLES
		vec4 texel0 = textureLod(frame, ss_txc - jitter0,0);
	
		vec4 texel1 = textureLod(pastHistory, ss_txc - ss_vel,0);

		// calc min-max of current neighbourhood
		vec2 uv = ss_txc;

	//if MINMAX_3X3 || MINMAX_3X3_ROUNDED

		vec2 du = vec2(renderDimensions.z, 0.0);
		vec2 dv = vec2(0.0, renderDimensions.w);

		vec4 ctl = textureLod(frame, uv - dv - du,0);
		vec4 ctc = textureLod(frame, uv - dv,0);
		vec4 ctr = textureLod(frame, uv - dv + du,0);
		vec4 cml = textureLod(frame, uv - du,0);
		vec4 cmc = textureLod(frame, uv,0);
		vec4 cmr = textureLod(frame, uv + du,0);
		vec4 cbl = textureLod(frame, uv + dv - du,0);
		vec4 cbc = textureLod(frame, uv + dv,0);
		vec4 cbr = textureLod(frame, uv + dv + du,0);

		vec4 cmin = min(ctl, min(ctc, min(ctr, min(cml, min(cmc, min(cmr, min(cbl, min(cbc, cbr))))))));
		vec4 cmax = max(ctl, max(ctc, max(ctr, max(cml, max(cmc, max(cmr, max(cbl, max(cbc, cbr))))))));

		//if MINMAX_3X3_ROUNDED || USE_YCOCG || USE_CLIPPING
			vec4 cavg = (ctl + ctc + ctr + cml + cmc + cmr + cbl + cbc + cbr) / 9.0;

		//if MINMAX_3X3_ROUNDED
			vec4 cmin5 = min(ctc, min(cml, min(cmc, min(cmr, cbc))));
			vec4 cmax5 = max(ctc, max(cml, max(cmc, max(cmr, cbc))));
			vec4 cavg5 = (ctc + cml + cmc + cmr + cbc) / 5.0;
			cmin = 0.5 * (cmin + cmin5);
			cmax = 0.5 * (cmax + cmax5);
			cavg = 0.5 * (cavg + cavg5);

		// clamp to neighbourhood of current sample
	//if USE_CLIPPING
		texel1 = clip_aabb(cmin.xyz, cmax.xyz, clamp(cavg, cmin, cmax), texel1);


		// feedback weight from unbiased luminance diff (t.lottes)
		float lum0 = Luminance(texel0.rgb);
		float lum1 = Luminance(texel1.rgb);
		float unbiased_diff = abs(lum0 - lum1) / max(lum0, max(lum1, 0.2));
		float unbiased_weight = 1.0 - unbiased_diff;
		float unbiased_weight_sqr = unbiased_weight * unbiased_weight;
		float k_feedback = mix(0.88, 0.97, unbiased_weight_sqr);

		// output
		return mix(texel0, texel1, k_feedback);
	}
		 
		          
 void main()                                                                         
 {  
	vec2 uv =TexCoord0;
	vec2 jitter0 = jitter.xy * renderDimensions.zw;

	vec4 tpos =matVP * vec4(uv.x*2.0-1.0,uv.y*2.0-1.0,0, 1.0);
	vec3 WorldPos = tpos.xyz/tpos.w;

	float dist =  texture2D(positions, uv).x;  
	vec3 eye = sinTime.yzw;
	vec3 rayDirection =normalize(WorldPos - eye );
	vec3 pos = eye+rayDirection*dist;

	//#if USE_DILATION
		//--- 3x3 nearest (good)
		vec3 c_frag = find_closest_fragment(uv);
		//vec2 ss_vel = textureLod(velocity, c_frag.xy,0).xy;
		//vec2 ss_vel =vec2(0,0);

	// reproject into previous frame
	vec4 rp_cs_pos = historyVP * vec4(pos,1.0);
	vec2 rp_ss_ndc = rp_cs_pos.xy / rp_cs_pos.w;
	vec2 rp_ss_txc = 0.5 * rp_ss_ndc + 0.5;

	// estimate static velocity
	vec2 ss_vel = (uv - rp_ss_txc);

		float vs_dist = c_frag.z;

	// temporal resolve
		vec4 color_temporal = temporal_reprojection(uv, ss_vel, vs_dist);

		// prepare outputs
		vec4 to_buffer = color_temporal;

		//if USE_MOTION_BLUR		

		float vel_mag = length(ss_vel / renderDimensions.zw);
		const float vel_trust_full = 2.0;
		const float vel_trust_none = 15.0;
		const float vel_trust_span = vel_trust_none - vel_trust_full;
		float trust = 1.0 - clamp(vel_mag - vel_trust_full, 0.0, vel_trust_span) / vel_trust_span;

		//if UNJITTER_COLORSAMPLES
		
			vec4 color_motion = sample_color_motion(frame, uv - jitter0, ss_vel);

		vec4 to_screen = mix(color_motion, color_temporal, trust);
	//else
	//	vec4 to_screen = color_temporal;

	//vec4 noise4 = PDsrand4(uv + sinTime.x + 0.6959174) / 510.0;
	vec4 noise4 = vec4(0,0,0,0);
		FragColor = clamp(to_screen+noise4,0.0,1.0);
		ivec2 storePos = ivec2(gl_FragCoord.xy);
		imageStore(newHistory, storePos, clamp(to_buffer+noise4 ,0.0,1.0));

 //FragColor = texture(frame,uv);
 //FragColor.x+=1000.0*length(ss_vel);
 //FragColor = vec4(,to_screen.z,1);
    //FragColor= vec4(0,1,0,1);                         
 }