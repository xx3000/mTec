 #version 330								  
																		 
 layout (location = 0) in vec2 Position;   
layout (location = 1) in vec2 TexCoord;   

uniform mat4 gWVP;						
uniform vec4 SpriteFrame;						
											 
out vec2 TexCoord0;							
											 
void main()									
{											
				
gl_Position =  gWVP * vec4(Position,0.0, 1.0);                                     
TexCoord0 = SpriteFrame.xy + (TexCoord * SpriteFrame.zw);									
				
}   