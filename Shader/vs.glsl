#version 420 core                                                  
			                                                                   
in vec4 position;                                                  
			                                                                   
out VS_OUT
{
    vec3 color;
    vec3 N;
} vs_out;                                                      
			                                                                   
uniform mat4 mv_matrix;                                            
uniform mat4 proj_matrix;                                          
			                                                                   
void main(void)                                                    
{                                                                  
    gl_Position = proj_matrix * mv_matrix * position;
    vs_out.N = position.xyz;    
}        