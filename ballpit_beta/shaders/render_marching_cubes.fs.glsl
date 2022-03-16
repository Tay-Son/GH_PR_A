#version 460 core

in GS_OUT
{
    vec3 N;
    vec3 L;
    vec3 V;
} fs_in;

out vec4 color;

uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 128.0;
uniform vec3 ambient = vec3(0.3, 0.3, 0.3);

void main(void)
{
    vec3 N = normalize(fs_in.N);
    vec3 L = normalize(fs_in.L);
    vec3 V = normalize(fs_in.V);

    vec3 R = reflect(-L, N);

    vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;
    vec3 specular = pow(max(dot(R, V), 0.0), specular_power) * specular_albedo;

    color = vec4(ambient + diffuse + specular, 1.0);
}
