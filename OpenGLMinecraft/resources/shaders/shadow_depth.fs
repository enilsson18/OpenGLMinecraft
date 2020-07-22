#version 330 core

layout(location = 0) out float fragmentdepth;

uniform float near_plane;
uniform float far_plane;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{             
    fragmentdepth = LinearizeDepth(gl_FragCoord.z) / far_plane;
    //fragmentdepth = gl_FragCoord.z;
}  