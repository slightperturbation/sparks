#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_fragColor;                 // interpolated color of fragment from vertex colors 
in vec2 f_texCoord;                  // texture coordinate of vertex
in vec4 f_vertex_screen;             // projected vertex position
in vec4 f_vertex_camera;             // unprojected vertex position
in mat4 f_projMat;
in mat4 f_modelViewMat;
in mat4 f_modelViewProjMat;

uniform sampler2D s_color;

// Non-standard uniforms
uniform vec4 u_color = vec4( 1, 1, 1, 1 );

void main()
{
	float cutoff = 0.001;

	outColor = u_color * texture( s_color, f_texCoord );

	// f_fragColor gives a measure of the intensity
	//outColor.a *= f_fragColor.a;

	if( outColor.a < cutoff )
	{
		discard;
	}

	//outColor = vec4( f_texCoord.t, 0.0, 1.0, 1.0 );
	//outColor.rgb *= smoothstep( cutoff, 1.0, outColor.a );
    //outColor = vec4( f_fragColor, 0.5 ) * texture( tex2d, f_texCoord );
    //outColor = vec4( f_fragColor.xyz, 0.5 );
    //outColor = f_fragColor;
    //if( f_texCoord.x < 0.1 || f_texCoord.x > 0.9 )
    
    	//outColor =  f_fragColor;
    
}