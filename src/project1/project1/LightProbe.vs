#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//out vec2 TexCoords;

void main()
{
    //TexCoords = aTexCoords;
	vec3 wp = vec3(model * vec4(aPos, 1.0));
	gl_Position =  projection * view * vec4(wp, 1.0);
	//gl_Position =  projection * vec4(0,0,-1, 1.0);
}