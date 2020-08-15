#version 330 core
layout(location = 0) out vec3 FragColor;

in vec3 FragPos;
in vec4 FragPosLightSpace;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightBrightness;
uniform float lightDistance;

uniform int projType;

uniform float near_plane;
uniform float far_plane;

uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    //perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    //scale the info to inbetween 0 and 1
    projCoords = projCoords * 0.5 + 0.5;
    //get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    //get depth from the lights perspective
    float currentDepth = projCoords.z;
    //see if the frag pos is in the shadow
    //bias fixes the peter panning effect

    float bias = 0;
    if (projType == 0){
    	bias = max(0.000002 * (dot(normalize(Normal), normalize(lightPos - FragPos))), 0.000002);
    }
    if (projType == 1){
    	//bias = max(0.00000001 * (dot(normalize(Normal), normalize(lightPos - FragPos))), 0.000000001);
        bias = max(0.000005 * (dot(normalize(Normal), normalize(lightPos - FragPos))), 0.0000005);
    }
    //float shadow = (currentDepth - bias) > closestDepth  ? 1.0 : 0.0;

    
    
    float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
    	for(int y = -1; y <= 1; ++y)
    	{
    	    float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
    	    shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
    	}    
	}
	shadow /= 9.0;
	

	if(projCoords.z > 1.0)
        shadow = 0.0;

    

    return shadow;
} 

void main()
{
    //shadow
    float shadow = ShadowCalculation(FragPosLightSpace);

    //disperse light over a distance and this is the factor of it
    float distance = distance(FragPos, lightPos);
    distance *= lightBrightness * (1/lightDistance);
    distance = 1 - distance;

	//combine and output lightings and shadows
    FragColor = vec3(1-shadow);
	//FragColor = vec3(111,0,0);
}