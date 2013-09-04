#version 150
//tissue_debug.frag

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_vertexPosition;
in vec4 f_fragColor; // interpolated color of fragment from vertex colors
in vec3 f_texCoord;  // texture coordinate of vertex
uniform float u_time;     // time in seconds
uniform float u_activationTime; 
// For Phong Lighting
in vec4 f_normal_camera;
in vec4 f_vertex_camera;


uniform usampler2D s_condition;
uniform sampler2D s_temperature;

void main()
{
    vec4 temp = texture( s_temperature, f_texCoord.xy );
    float t = (temp.r - 37.0)/63.0;
	outColor = vec4( t,t,t, 1.0 );

    uvec4 cond4 = texture( s_condition, f_texCoord.xy );
	uint cond = cond4.r;
    /// normalTissue=0, dessicatedTissue=1, vaporizingTissue=2, charredTissue=3.
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
    return;
}