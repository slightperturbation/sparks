#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_fragColor; // interpolated color of fragment from vertex colors 
in vec3 f_texCoord;  // texture coordinate of vertex
in vec4 f_vertexPosition;

uniform sampler3D s_density3d;

void main()
{
    //outColor = f_fragColor * texture( s_density3d, f_texCoord ).x ;
    outColor = vec4( f_fragColor.rgb, texture( s_density3d, f_texCoord )[0] );
}
