#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
flat in int TexNum;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

void main()
{
	//load textures sorry I am too lazy to figure out how to make and arrange arrays in glsl
	vec3 objectColor;

	if (TexNum == 0){
		objectColor = vec3(texture(texture0, TexCoord));
	}
    else if (TexNum == 1){
		objectColor = vec3(texture(texture1, TexCoord));
	}
	else if (TexNum == 2){
		objectColor = vec3(texture(texture2, TexCoord));
	}
	else if (TexNum == 3){
		objectColor = vec3(texture(texture3, TexCoord));
	}
	//ambient lighting
	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

	//diffuse lighting
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	//combine and output lightings
	vec3 result = (ambient + diffuse) * objectColor;
	FragColor = vec4(result, 1.0);
}