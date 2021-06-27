#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float texNum;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
flat out int TexNum;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = transpose(inverse(mat3(model))) * aNormal;
    //add one since we need to make room for the shadow map.
    TexNum = int(texNum);

    //if (FragPosLightSpace == vec4(0.0)){
    	//TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    //}

    if (TexNum >= 0){
        //gl_Position = vec4(0);
    }
}