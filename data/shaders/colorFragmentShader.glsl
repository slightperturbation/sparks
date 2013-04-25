#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
out vec4 f_fragColor; // interpolated color of fragment from vertex colors 
out vec3 f_texCoord;  // texture coordinate of vertex
out vec4 f_vertexPosition;

//uniform sampler2D tex2d;
//uniform sampler3D tex3d;

void main()
{
    outColor = vec4(1,0,0,1); //vec4( TexCoord, 1.0 );
}