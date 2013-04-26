#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_fragColor; // interpolated color of fragment from vertex colors 
in vec2 f_texCoord;  // texture coordinate of vertex
in vec4 f_vertexPosition;

uniform sampler2D catTex;
//uniform sampler2D tex2d;
//uniform sampler3D tex3d;

void main()
{
	// Test 2d texture coordinates are set:
	//outColor = vec4( f_texCoord.s, 0.0, f_texCoord.t, 1.0 );

	// Show interpolated vertex color
	//outColor = vec4( f_fragColor.s, 0.0, f_fragColor.t, 1.0 );

    outColor = vec4( texture( catTex, f_texCoord ), 1.0 );
     //f_fragColor;//vec4(1,1,0,1); //vec4( TexCoord, 1.0 );
}