#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_fragColor; // interpolated color of fragment from vertex colors 
in vec2 f_texCoord;  // texture coordinate of vertex
in vec4 f_vertexPosition;

//////////////////////////////////////////////////////////////////////
// Common Uniforms (see RenderCommand)
uniform mat4 u_projViewModelMat;     // projection * view * model
uniform mat4 u_viewModelMat;         // transforms object into camera(eye) space
uniform mat4 u_inverseViewModelMat;  // inverse of the model-view matrix, can give camera position
uniform mat4 u_projMat;              // projects camera(eye) space to clip(screen) space
uniform mat3 u_normalMat;            // transpose(inverse(viewModelMat))
uniform float u_time;                // current time (in seconds) 
//////////////////////////////////////////////////////////////////////

// Non-standard uniforms
uniform vec4 u_color;

uniform sampler2D s_color;
//uniform sampler2D s_color2;
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

///	f_fragColor * 
//outColor = vec4( f_texCoord, 1.0, 1.0 );
	outColor = vec4( texture( s_color, f_texCoord ).rgb, 1.0 );
	
// HACK/TEST to force transparency in black regions
	if( outColor.r + outColor.g + outColor.b < 0.01 )
	{
		discard;
	}
	//outColor = texture( s_color, f_texCoord );
	//outColor = vec4( texture( s_color2, f_texCoord ).xyz, 1.0 );
	
	//outColor = vec4( f_texCoord, 1.0,  1.0 );
	// if( outColor.a < cutoff )
	// {
	// 	discard;
	// }
    //outColor = mix( texture( s_color, f_texCoord ), texture( s_color2, f_texCoord ), 0.1 );
     //f_fragColor;//vec4(1,1,0,1); //vec4( TexCoord, 1.0 );
}