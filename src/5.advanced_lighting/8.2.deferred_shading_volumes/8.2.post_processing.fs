#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gTexture;
uniform sampler2D gDepth;

void main()
{             
    // retrieve data from gbuffer

	vec2 texelSize = 1.0 / vec2(textureSize(gTexture, 0));

    vec3 FragPos = texture(gTexture, TexCoords).rgb;

	int blur_s = 2;

    // then calculate lighting as usual
	vec4 colors = vec4(0,0,0,0);

	float d1 = textureOffset(gDepth, TexCoords, ivec2(blur_s,blur_s)).y;
	float d2 = textureOffset(gDepth, TexCoords, ivec2(blur_s,blur_s)).y;
	float d3 = textureOffset(gDepth, TexCoords, ivec2(blur_s,blur_s)).y;
	float d4 = textureOffset(gDepth, TexCoords, ivec2(blur_s,blur_s)).y;

	colors = vec4(d1,d2,d3,1.0);

	vec4 depthPos = textureOffset(gDepth, TexCoords, ivec2(0, 0));

	vec3 pos = depthPos.xyz / depthPos.w;

	
	//colors += textureOffset(gTexture, TexCoords, ivec2(blur_s,blur_s));
	//colors += textureOffset(gTexture, TexCoords, ivec2(-blur_s,blur_s));
	//colors += textureOffset(gTexture, TexCoords, ivec2(blur_s,-blur_s));
	//colors += textureOffset(gTexture, TexCoords, ivec2(-blur_s,-blur_s));

	//colors += texture(gTexture, TexCoords + vec2(blur_s,blur_s)*texelSize);
	//colors += texture(gTexture, TexCoords + vec2(-blur_s,blur_s)*texelSize);
	//colors += texture(gTexture, TexCoords + vec2(blur_s,-blur_s)*texelSize);
	//colors += texture(gTexture, TexCoords + vec2(-blur_s,-blur_s)*texelSize);

    FragColor = vec4(pos.xyz, 1.0);//colors / 4; //texture(gTexture, TexCoords);

}
