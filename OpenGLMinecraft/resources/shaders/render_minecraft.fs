#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
flat in int TexNum;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightBrightness;
uniform float lightDistance;

uniform float near_plane;
uniform float far_plane;

uniform float shadow_near;
uniform float shadow_far;


uniform int pcflevel;

uniform samplerCube shadowMap;
uniform sampler2D shadowBlurMap;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform sampler2D texture6;

float ShadowCalculation(float distance)
{
    //get vector between fragment position and light position
    vec3 fragToLight = FragPos - lightPos;

    //ise the fragment to light vector to sample from the depth map    
    //float closestDepth = texture(shadowMap, fragToLight).r;

    //it is currently in linear range between [0,1], let's re-transform it back to original depth value
    //closestDepth *= shadow_far/shadow_near;

    //now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);

    //bias fixes the peter panning effect
    float bias = 0;
    //bias = max(0.00000001 * (dot(normalize(Normal), normalize(lightPos - FragPos))), 0.000000001);
    //bias = max(0.000005 * (dot(normalize(Normal), normalize(lightPos - FragPos))), 0.0000005);
    bias = max(1.0 * (dot(normalize(Normal), normalize(lightPos - FragPos))), 0.000005);
    //bias = 2;
    //float shadow = (currentDepth - bias) > closestDepth  ? 1.0 : 0.0;

    /*
    //int pcfLevel = pcflevel * textureSize(shadowMap,0).x/1024;
    //int pcfLevel = int(pcflevel * (1-distance));
    int pcfLevel = pcflevel;
    float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -pcfLevel; x <= pcfLevel; ++x)
	{
    	for(int y = -pcfLevel; y <= pcfLevel; ++y)
    	{
    	    float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
    	    shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
    	}    
	}
	shadow /= (pcfLevel*2 + 1) * (pcfLevel*2 + 1);
	*/

    vec3 sampleOffsetDirections[20] = vec3[]
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );  

    float shadow = 0.0;
    int samples  = 20;
    float diskRadius = 0.1;
    //float diskRadius = (1 - distance) * 0.1; 
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= shadow_far/shadow_near;
        if(currentDepth - bias > closestDepth){
            shadow += 1.0;
        }
    }
    shadow /= float(samples);  

	//if(projCoords.z > 1.0)
        //shadow = 0.0;

    
    //FragColor = vec4(vec3(closestDepth * (shadow_near/shadow_far)), 1.0);
    //FragColor = vec4(vec3(texture(shadowMap, fragToLight).r), 1.0);
    return shadow;
} 

void main()
{
	//load textures sorry I am too lazy to figure out how to make and arrange arrays in glsl
	vec3 objectColor;

    if (TexNum == 0){
		objectColor = vec3(texture(texture1, TexCoord));
	}
	else if (TexNum == 1){
		objectColor = vec3(texture(texture2, TexCoord));
	}
	else if (TexNum == 2){
		objectColor = vec3(texture(texture3, TexCoord));
	}
	else if (TexNum == 3){
		objectColor = vec3(texture(texture4, TexCoord));
	}
	else if (TexNum == 4){
		//objectColor = vec3(texture(texture5, TexCoord));
	}
	else if (TexNum == 5){
		//objectColor = vec3(texture(texture6, TexCoord));
	}
	//objectColor = vec3(255 - 255 * texture(shadowMap, (vec3(FragPosLightSpace.xyz / FragPosLightSpace.w) * 0.5 + 0.5).xy).r);
	//objectColor = vec3(255 - 255 * (vec3(FragPosLightSpace.xyz / FragPosLightSpace.w) * 0.5 + 0.5).z);
	//objectColor = vec3(texture(shadowMap, TexCoord));
    //objectColor = vec3(255 * ShadowCalculation(1));

	float diff = 0;

	//ambient lighting
	float ambientStrength = 0.05;
    vec3 ambient = ambientStrength * lightColor;

    //diffuse lighting
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

    //specular lighting
    //basically just make the surface brighter if more light reflects more into the viewers eyes
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor; 

	//disperse light over a distance and this is the factor of it
    float distance = distance(FragPos, lightPos);
    distance *= lightBrightness * (1/lightDistance);
    distance = 1 - distance;
    //distance *= 2;

    //shadow
    float shadow = ShadowCalculation(distance);
    if (shadow <= 0){
    	//shadow = texture(shadowBlurMap, (vec3(FragPosLightSpace.xyz / FragPosLightSpace.w) * 0.5 + 0.5).xy).r;
    }
    shadow = 1 - shadow * 1;

    //there was a glitch where the distance was darker than the shadow so the shadows were actually lighter if you went far enough away.
    float darkness = min(shadow, distance);
	//combine and output lightings and shadows
	vec3 result = ((darkness) * (specular + diffuse) * 1 + ambient * 1) * objectColor;
	FragColor = vec4(result, 1.0);
    //FragColor = vec4(vec3(0.0), 1.0);

	if (shadow == 0.0f){
		//FragColor = vec4(1.0);
	}
}