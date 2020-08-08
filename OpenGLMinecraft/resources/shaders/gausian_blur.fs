#version 330 core

out vec3 FragColor;

in vec2 blurTexCoords[21];
in vec2 TexCoords;

uniform sampler2D tex;
uniform float near_plane;
uniform float far_plane;

void main()
{   
	//output the color and sample from all the surrounding textures
	FragColor = vec3(0.0);

	//add the value scaled down based on the number of extra samples we are drawing from
	//if you were wondering i access the r value since it is the first value of the vec4 and I am inputing a single value float map into is so only r in rbg is nonzero
	for (int i = 0; i < 21; i++){
		FragColor += (texture(tex, blurTexCoords[i]) * 0.05).r;
	}

	//output *= 20;
	//output = 1.0;
	//output = 0.0;

	FragColor *= 255;

	//FragColor = vec4(0.0);
	FragColor = vec3(texture(tex, TexCoords).r);
}  