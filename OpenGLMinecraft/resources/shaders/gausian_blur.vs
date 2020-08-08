#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 blurTexCoords[21];
out vec2 TexCoords;

uniform float textureWidth;
uniform float textureHeight;

void main()
{
    gl_Position = vec4(aPos, 1.0);

    vec2 center = aPos.xy * 0.5 + 0.5;

    float pixWidth = 1.0 / textureWidth;
    float pixHeight = 1.0 / textureHeight;

    //x values
    for (int i = -5; i <= 5; i++){
    	//set all the blur values based on the center pos
    	blurTexCoords[i + 5] = center + vec2(pixWidth * i, 0.0);
    }

    //y values
    for (int i = -5; i <= 5; i++){
    	//set all the blur values based on the center pos
    	blurTexCoords[i + 5 + 10] = center + vec2(0.0, pixHeight * i);
    }

    TexCoords = center;
    //TexCoords = aTexCoord;
}  