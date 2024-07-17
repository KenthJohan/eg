#version 430

uniform vec4 vs_params[5];
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;
layout(location = 3) in vec4 color0;
layout(location = 4) in vec3 inst_pos;

layout(location = 0) out vec4 color;

void main()
{
	gl_Position = mat4(vs_params[0], vs_params[1], vs_params[2], vs_params[3]) * vec4(position.xyz + inst_pos, 1.0);
	if (vs_params[4].w == -1.0) {
		color = vec4((normal + vec3(1.0)) * 0.5, 1.0);
	} else if (vs_params[4].w == -2.0) {
		color = vec4(texcoord, 0.0, 1.0);
	} else if (vs_params[4].w == -3.0) {
		color = color0;
	} else if (vs_params[4].w == -4.0) {
		color = vec4(1,0,0,1);
	} else if (vs_params[4].w > 0) {
		color = vs_params[4];
	}
}