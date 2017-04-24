#version 330                                                                  

 in vec2 TexCoord0;                                                                  
 
 out vec4 FragColor;                                                                 
 
 uniform sampler2D Texture0;                                                         
 uniform sampler2D Texture1; 
 uniform int texnr; 
 void main()                                                                         
 {                                                                                   
	vec4 texel0, texel1, resultColor;					
	if(texnr == 2) 
	 {    
		texel0 = texture2D(Texture0, TexCoord0);			
		texel1 = texture2D(Texture1, TexCoord0);			
		resultColor = mix(texel0, texel1, texel1.a);    
	}
	else 
	{    
		resultColor=texture2D(Texture0, TexCoord0.xy);  
	}
										
	FragColor = resultColor;                             
 }