#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aCoord;

out vec3 Pos;
out vec3 Normal;
out vec2 Coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_postion = aPos * model * view * projection;
	Pos = vec3(model * vec4(aPos, 1.0));

	Normal = aNormal
}