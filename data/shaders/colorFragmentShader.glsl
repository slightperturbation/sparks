#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_fragColor; // interpolated color of fragment from vertex colors 
in vec2 f_texCoord;  // texture coordinate of vertex
in vec4 f_vertexPosition;
// float f_time;     // time in seconds

// Samplers are named s_TEXTURENAME
uniform sampler2D s_color;
uniform sampler2D s_color2;
//uniform sampler2D s_tex2d;
//uniform sampler3D s_tex3d;

void main()
{
	// Test 2d texture coordinates are set:
	//outColor = vec4( f_texCoord.s, 0.0, f_texCoord.t, 1.0 );

	// Show interpolated vertex color
	//outColor = vec4( f_fragColor.s, 0.0, f_fragColor.t, 1.0 );

	//ivec2 x = textureSize( s_color, 0 );
	//outColor = vec4( x.x/4.0, x.y/4.0, 0.0, 1.0 );  
	outColor = vec4( texture( s_color2, f_texCoord ).xyz, 1.0 );
	
	//outColor = vec4( f_texCoord, 1.0,  1.0 );
	// if( outColor.a < cutoff )
	// {
	// 	discard;
	// }
    //outColor = mix( texture( s_color, f_texCoord ), texture( s_color2, f_texCoord ), 0.1 );
     //f_fragColor;//vec4(1,1,0,1); //vec4( TexCoord, 1.0 );
}