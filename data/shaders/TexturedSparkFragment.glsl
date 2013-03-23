#version 150

out vec4 outColor;
in vec3 FragColor; // name must match with vertex shader out-attrib
in vec3 TexCoord;  // name must match with vertex shader out-attrib
in vec4 vertexPosition;
in mat4 ModelViewProj;

//uniform sampler2D tex2d;
uniform sampler3D tex3d;

void main()
{
    outColor = vec4( TexCoord, 1.0 );
}