#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos;

void main()
{
	//renders to each face
    for(int face = 0; face < 6; ++face)
    {
    	//built-in variable that specifies to which face we render.
        gl_Layer = face;

        //for each triangle vertex emit it to the next shader
        for(int i = 0; i < 3; ++i)
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  