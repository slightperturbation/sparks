#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_fragColor; // interpolated color of fragment from vertex colors 
in vec2 f_texCoord;  // texture coordinate of vertex
in vec2 f_blurTexCoords[14]; // Gaussian sample points

//////////////////////////////////////////////////////////////////////
// Common Uniforms (see RenderCommand)
uniform mat4  u_projViewModelMat;     // projection * view * model
uniform mat4  u_viewModelMat;         // transforms object into camera(eye) space
uniform mat4  u_inverseViewModelMat;  // inverse of the model-view matrix, can give camera position
uniform mat4  u_projMat;              // projects camera(eye) space to clip(screen) space
uniform mat3  u_normalMat;            // transpose(inverse(viewModelMat))
uniform float u_time;                 // current time (in seconds) 
uniform vec2  u_targetSizeInPixels;   // size in pixels of the current render target
//////////////////////////////////////////////////////////////////////

uniform vec4 u_color = vec4( 1, 1, 1, 1 );
uniform sampler2D s_color;

void main()
{
    outColor = vec4(0.0);
    outColor += texture(s_color, f_blurTexCoords[ 0])*0.008846951;
    outColor += texture(s_color, f_blurTexCoords[ 1])*0.018215906;
    outColor += texture(s_color, f_blurTexCoords[ 2])*0.033562395;
    outColor += texture(s_color, f_blurTexCoords[ 3])*0.055335035;
    outColor += texture(s_color, f_blurTexCoords[ 4])*0.081638025;
    outColor += texture(s_color, f_blurTexCoords[ 5])*0.107777932;
    outColor += texture(s_color, f_blurTexCoords[ 6])*0.127324582;
    outColor += texture(s_color, f_texCoord         )*0.134598348;
    outColor += texture(s_color, f_blurTexCoords[ 7])*0.127324582;
    outColor += texture(s_color, f_blurTexCoords[ 8])*0.107777932;
    outColor += texture(s_color, f_blurTexCoords[ 9])*0.081638025;
    outColor += texture(s_color, f_blurTexCoords[10])*0.055335035;
    outColor += texture(s_color, f_blurTexCoords[11])*0.033562395;
    outColor += texture(s_color, f_blurTexCoords[12])*0.018215906;
    outColor += texture(s_color, f_blurTexCoords[13])*0.008846951;
    outColor *= u_color;
}