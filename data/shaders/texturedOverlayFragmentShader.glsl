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

uniform vec4 u_color = vec4( 1, 1, 1, 1 );

uniform sampler2D s_color;

void main()
{
    vec2 flippedCoord = vec2( f_texCoord.s, 1.0 - f_texCoord.t );
    vec4 tex = texture( s_color, flippedCoord );
    outColor = u_color * tex;
    
    if( outColor.a < 0.1 )
    {
        discard;
    }
}