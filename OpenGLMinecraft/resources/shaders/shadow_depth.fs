#version 330 core

layout(location = 0) out float fragmentdepth;

uniform float near_plane;
uniform float far_plane;
uniform int projType;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{   
	if (projType == 1){
    	fragmentdepth = LinearizeDepth(gl_FragCoord.z) / far_plane;
    }
    if (projType == 0){
    	fragmentdepth = gl_FragCoord.z;
    }
}  