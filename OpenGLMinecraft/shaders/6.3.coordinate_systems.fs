#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
flat in int TexNum;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

void main()
{
	if (TexNum == 0){
		FragColor = texture(texture0, TexCoord);
	}
    else if (TexNum == 1){
		FragColor = texture(texture1, TexCoord);
	}
	else if (TexNum == 2){
		FragColor = texture(texture2, TexCoord);
	}
	else if (TexNum == 3){
		FragColor = texture(texture3, TexCoord);
	}
}