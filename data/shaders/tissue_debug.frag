#version 150
//tissue_debug.frag

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
uniform float u_time;     // time in seconds
uniform float u_activationTime; 

// Out to fragment shader
out vec4 f_fragColor;      // interpolated color of fragment from vertex colors 
out vec2 f_texCoord;       // texture coordinate of vertex
out vec4 f_vertex_screen;  // Projected vertex into the clip-space
out vec4 f_normal_camera;  // For phong lighting
out vec4 f_vertex_camera;  // For phong lighting
out vec4 f_vertexPosition;
out vec4 f_shadowPosition; // Position of light


uniform usampler2D s_condition;
uniform sampler2D s_temperature;
uniform sampler2D s_depthMap;

void main()
{
    vec4 temp = texture( s_temperature, f_texCoord.xy );
    float t = 0.25 + (temp.r - (37.0+273.15))/(200.0+273.15);
	outColor = vec4( t, t, t, 1.0 );

    uint centerCond = texture( s_condition, f_texCoord ).r;

    uint cond = centerCond;
	if( cond == 0u )
	{
		outColor *= vec4( 1, 0, 0, 1 );
	}
	if( cond == 1u )
	{
		outColor *= vec4( 0, 1, 0, 1 );
	}
	if( cond == 2u ) 
	{
		outColor *= vec4( 0, 0, 1, 1 );
	}
	if( cond == 3u )
	{
		outColor *= vec4( 0, 1, 1, 1 );
	}
	if( cond == 4u )
	{
		outColor *= vec4( 1, 1, 0, 1 );
	}
}