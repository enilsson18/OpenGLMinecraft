#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float near_plane;
uniform float far_plane;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by adjusting accoring to near and far planes
    lightDistance = lightDistance * (near_plane/far_plane);
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
    //gl_FragDepth = 1;
}