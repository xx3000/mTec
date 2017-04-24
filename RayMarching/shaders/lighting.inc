// Copyright (c) <2017> Michael Mroz
// This file is subject to the MIT License as seen in the root of this folder structure (LICENSE.TXT)
 vec3 shading(vec3 col, vec3 pos, vec3 normal, float thickness, vec3 dir , vec3 lightDir, float shadow, float occlusion, int id)
{
	float ambient = 0.2;
	ambient *=occlusion;

	// GDC 2011 – Approximating Translucency for a Fast, Cheap and Convincing Subsurface Scattering Look
	float fLTAmbient = 0.2;
	int iLTPower = 4;
	float fLTDistortion = 0.2;
	float fLTScale = 1.0;
	//float fLightAttenuation = dot(normal,lightDir)+dot(dir,-lightDir);
	float fLightAttenuation = 1.0;
	
    float dist =length( pos-lightPos);
    float att = 0.2+dist*0.1+0.03*dist*dist;

	float diffuse = clamp( dot( lightDir ,normal ), 0.0, 1.0 );

    float fLT =0.0;
	if(thickness >0)
	{
		vec3 vLTLight = lightDir +normal *fLTDistortion;
		float fLTDot = pow(clamp(dot(dir,-vLTLight),0,1), iLTPower) *fLTScale;
		fLT = fLightAttenuation * (fLTDot ) *thickness;
	}

	vec3 tintedCol =col;
	SDF obj =objects[id];
	tintedCol*=obj.color.xyz;
	vec4 params = obj.lightParams;

	vec3 l = mix(tintedCol,obj.fillColor.xyz*col,fLT);
	
	vec3 view = dir;
	vec3 h = normalize(view+lightDir);
    float specAngle = max(dot(h, normal), 0.0);
	float lightCut = smoothstep(.0,.1,diffuse);
	float specPower = exp2(params.x);
	float specular = pow(specAngle*lightCut, specPower)*specPower/params.y;

	vec3 light=vec3(0);
	light+=shadow*diffuse;
	light+=ambient;
	light+= fLT;
	light+=shadow*specular;

	return (light*l)/att;
}

// Copyright https://learnopengl.com/#!PBR/Lighting
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

vec3 shadingPBR(vec3 col, vec3 pos, vec3 normal, float thickness, vec3 dir , vec3 lightDir, float shadow, float occlusion, int id, float roughness, float metallic, vec3 reflection)
{
	SDF obj =objects[id];
	vec3 albedo=col*obj.color.xyz;

	vec3 lightColor = vec3(1.0, 0.751, 0.673);
	vec3 lightIntensity=lightColor*207.35;

	vec3 N = normal;
    vec3 V = dir;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);

    // calculate per-light radiance
    vec3 L = lightDir*1.0;
    vec3 H = normalize(V + L);
    float distance    = length(lightPos - pos);
    float attenuation = 1.0 / (distance * distance);
      
      
	  // GDC 2011 – Approximating Translucency for a Fast, Cheap and Convincing Subsurface Scattering Look
	float fLTAmbient = 0.01;
	int iLTPower = 6;
	float fLTDistortion = 0.2;
	float fLTScale = 1.0;
	//float fLightAttenuation = dot(normal,lightDir)+dot(dir,-lightDir);
	float fLightAttenuation = 1.0;
	//float fLightAttenuation = attenuation;
	  
	  float fLT =0.0;
	 if(thickness > 0)
	{
		vec3 vLTLight = lightDir +normal *fLTDistortion;
		float fLTDot = pow(clamp(dot(dir,-vLTLight),0,1), iLTPower) *fLTScale;
		fLT = fLightAttenuation * (fLTDot+fLTAmbient ) *thickness;
	}
	//lightColor+=fLT*50.0;
	albedo = mix(albedo,albedo*lightIntensity*0.25,fLT); 
	// albedo = mix(albedo,lightColor,fLT);    
	
	
	vec3 radiance     = lightIntensity * attenuation;      
    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
        
    vec3 nominator    = NDF * G * F;
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; 
    vec3 specular     = (nominator / denominator);
          
	float tf =(fLT*fLT*fLT)*lightColor*col;
	//return vec3(tf,tf,tf);   
    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);                
    Lo += ((kD * albedo) / PI + specular) *shadow* radiance * NdotL; 
   
	
    vec3 ambient = vec3(0.03) * albedo * occlusion;
    vec3 color = ambient + Lo + tf;
	
    //color = pow(color, vec3(1.0/2.2));  
   
    return color;
}