#version 150

in vec3 position;
in vec3 inVertexColor;
in vec3 texCoord3d;
uniform mat4 modelViewProj; // projection * view * model
out vec3 FragColor; // name must match with fragment shader in-attrib
out vec3 TexCoord;  // texture coordinate of vertex
out vec4 vertexPosition;
out mat4 ModelViewProj;

void main()
{
    ModelViewProj = modelViewProj;
    FragColor = inVertexColor;
    TexCoord = texCoord3d;// compute texcoord from 3d coordinate  
    vertexPosition = modelViewProj * vec4( position, 1.0 );
    gl_Position = modelViewProj * vec4( position, 1.0 );
}
