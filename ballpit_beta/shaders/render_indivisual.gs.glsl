#version 460 core

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 60) out;

layout (std140) uniform constants{
    mat4 mv_matrix;
    mat4 view_matrix;
    mat4 proj_matrix;
};

uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);


uniform vec3 icos_vertices[12];

uniform int icos_indices[60];

out GS_OUT{
    vec3 N;
    vec3 L;
    vec3 V;
} gs_out;

void make_face(int i){
	vec4 P = mv_matrix * (gl_in[0].gl_Position + vec4(icos_vertices[i], 1.0));
	gs_out.N = mat3(mv_matrix) * icos_vertices[i];
	gs_out.L = light_pos - P.xyz;
	gs_out.V = -P.xyz;

	gl_Position = proj_matrix * P;
	EmitVertex();
}

void main(){
	int i;
	int j;
	int count;
	for(i=0,count=0;i<20;i++){
		for(j=0;j<3;j++,count++){
			make_face(icos_indices[count]);
		}
		EndPrimitive();
	}
	EndPrimitive();
}
