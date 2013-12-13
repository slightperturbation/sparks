#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_fragColor; // interpolated color of fragment from vertex colors 
in vec2 f_texCoord;  // texture coordinate of vertex

//////////////////////////////////////////////////////////////////////
// Common Uniforms (see RenderCommand)
uniform mat4 u_projViewModelMat;     // projection * view * model
uniform mat4 u_viewModelMat;         // transforms object into camera(eye) space
uniform mat4 u_inverseViewModelMat;  // inverse of the model-view matrix, can give camera position
uniform mat4 u_projMat;              // projects camera(eye) space to clip(screen) space
uniform mat3 u_normalMat;            // transpose(inverse(viewModelMat))
uniform float u_time;                // current time (in seconds) 
//////////////////////////////////////////////////////////////////////

uniform vec4 u_contactColor = vec4( 0.7, 0.7, 0.7, 1 );
uniform vec4 u_hoverColor = vec4( 0.1, 0.1, 0.1, 0.3 );
uniform vec4 u_nothingColor = vec4( 0.1 );
uniform float u_lowerLevelFraction = 0.5;
uniform float u_upperLevelFraction = 0.75;
uniform sampler2D s_color;

void main()
{
    vec2 flippedCoord = vec2( f_texCoord.s, 1.0 - f_texCoord.t );
    float height = texture( s_color, flippedCoord ).r;

    // Units of height are in fraction of distance from near plane to far plane
    if( height > u_upperLevelFraction )
    {
    	outColor = u_nothingColor * vec4( (-1.0/(1.0-u_upperLevelFraction))*height + (1.0/(1.0-u_upperLevelFraction))  );
    }
    else if( height > u_lowerLevelFraction )
    {
	    outColor = u_hoverColor * vec4( vec3(height), 0.6 );
    }
    else
    {
    	outColor = u_contactColor;
    }
}