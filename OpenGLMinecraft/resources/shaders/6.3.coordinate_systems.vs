#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float texNum;

out vec2 TexCoord;
out vec3 FragPos;
out vec4 FragPosLightSpace;
out vec3 Normal;
flat out int TexNum;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = transpose(inverse(mat3(model))) * aNormal;
    //add one since we need to make room for the shadow map.
    TexNum = int(texNum) + 1;

    //make shadows light space
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    //if (FragPosLightSpace == vec4(0.0)){
    	//TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    //}
}