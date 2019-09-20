#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;

void main()
{

	//mat2 rotate = mat2(cos(Normal), -sin(Normal), sin(Normal), cos(Normal));
	//mat2 rotate = mat2(0,-1, 1,0);
	//mat2 rotate = mat2(cos(1.5707),-sin(1.5707), sin(1.5707),cos(1.5707));
	//vec2 coord = gl_PointCoord;

	//FragColor = vec4(0,1.0,0,1.0);//texture(texture1, gl_PointCoord);
	FragColor = texture(texture1, TexCoord);
}