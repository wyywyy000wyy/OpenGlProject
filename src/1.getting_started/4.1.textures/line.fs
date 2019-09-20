#version 330 core
out vec4 FragColor;

// texture sampler
uniform sampler2D texture1;

void main()
{
	//FragColor = vec4(0,1.0,0,1.0);//texture(texture1, gl_PointCoord);
	FragColor = texture(texture1, gl_PointCoord);
}