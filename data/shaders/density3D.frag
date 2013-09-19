#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_fragColor; // interpolated color of fragment from vertex colors 
in vec3 f_texCoord;  // texture coordinate of vertex
in vec4 f_vertexPosition;

uniform sampler3D s_density3d;

// Blur samples
in vec3 f_blurTexCoords[6];

void main()
{
	float scale = 1.0/9.0;
	float avgDensity = 0;
	avgDensity += (1.0/3.0) * texture( s_density3d, f_texCoord )[0];
	avgDensity += scale     * texture( s_density3d, f_blurTexCoords[0] )[0];
	avgDensity += scale     * texture( s_density3d, f_blurTexCoords[1] )[0];
	avgDensity += scale     * texture( s_density3d, f_blurTexCoords[2] )[0];
	avgDensity += scale     * texture( s_density3d, f_blurTexCoords[3] )[0];
	avgDensity += scale     * texture( s_density3d, f_blurTexCoords[4] )[0];
	avgDensity += scale     * texture( s_density3d, f_blurTexCoords[5] )[0];
	
	outColor = vec4( f_fragColor.rgb, avgDensity );

    //outColor = f_fragColor * texture( s_density3d, f_texCoord ).x ;
    //outColor = vec4( f_fragColor.rgb, texture( s_density3d, f_texCoord )[0] );
}
