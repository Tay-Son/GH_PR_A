#version 420 core                 
			                                                            
out vec4 color;                                                    
			                                                                   
in VS_OUT
{
    vec3 color;
    vec3 N;
} fs_in;                                                      

uniform vec3 diffuse_albedo = vec3(1., 0.2, 0.2);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 200.0;

uniform vec3 Lu = vec3(30.0,30.0,30.0);
uniform vec3 Vu = vec3(0.0,0.0,10.0);


void main(void)                                                    
{                   
        // Normalize the incoming N, L and V vectors
    vec3 N = normalize(fs_in.N);
    vec3 L = normalize(Lu);
    vec3 V = normalize(Vu);
    vec3 H = normalize(L + V);

    // Compute the diffuse and specular components for each fragment
    vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;
    vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;

    // Write final color to the framebuffer
    color = vec4(diffuse + specular, 1.0);
}                                                                  