#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNor;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


out vec2 TexCoords;
out vec3 WorldPos;
out vec3 WorldNormal;

void main()
{
    TexCoords = aTexCoords;
	WorldPos = vec3(model * vec4(aPos, 1.0));
	WorldNormal = mat3(model) * aNor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    //gl_Position = projection * view * vec4(aPos, 1.0);
}