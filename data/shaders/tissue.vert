#version 150

// MeshVertex attributes (see Mesh.hpp)
in vec3 v_position;
in vec3 v_normal;
in vec4 v_color;
in vec3 v_texCoord;

//////////////////////////////////////////////////////////////////////
// Common Uniforms (see RenderCommand)
uniform mat4 u_projViewModelMat;     // projection * view * model
uniform mat4 u_viewModelMat;         // transforms object into camera(eye) space
uniform mat4 u_inverseViewModelMat;  // inverse of the model-view matrix, can give camera position
uniform mat4 u_projMat;              // projects camera(eye) space to clip(screen) space
uniform mat3 u_normalMat;            // transpose(inverse(viewModelMat))
uniform float u_time;                // current time (in seconds) 
uniform float u_activationTime;
//////////////////////////////////////////////////////////////////////

// Out to fragment shader
out vec4 f_fragColor;      // interpolated color of fragment from vertex colors 
out vec2 f_texCoord;       // texture coordinate of vertex
out vec4 f_vertex_screen;  // Projected vertex into the clip-space
out vec4 f_normal_camera;  // For phong lighting
out vec4 f_vertex_camera;  // For phong lighting
out vec4 f_vertexPosition;
out vec4 f_shadowPosition; // Position of light

struct ShadowLight 
{
    mat4 projViewModelMat;
    vec4 color;
};

uniform ShadowLight u_shadowLight[4];
uniform int u_currLightIndex = 0;
uniform vec2 u_textureRepeat = vec2(1,1);

// blurred texture coords for smoothing the tissue map
out vec2 f_blurTexCoords[25];
// Blur radius == this should be tied directly to the 
// size of the tissue condition map!
// 1/map_size
uniform float u_blurRadius = 1.0/512.0; 

void main()
{
    f_vertexPosition = vec4( v_position, 1.0 );
	f_normal_camera = vec4( u_normalMat * v_normal, 0.0 ); // dir
	f_vertex_camera = u_viewModelMat * vec4( v_position, 1.0 ); // point
    f_fragColor = v_color ;
    f_texCoord = u_textureRepeat * v_texCoord.st;  
    f_vertex_screen = u_projViewModelMat * vec4( v_position, 1.0 );
    f_shadowPosition = u_shadowLight[u_currLightIndex].projViewModelMat * vec4( v_position, 1.0 );
    gl_Position = f_vertex_screen;


    float scale = u_blurRadius;
    f_blurTexCoords[ 0] = f_texCoord.xy + vec2(-2*scale, -2*scale);
    f_blurTexCoords[ 1] = f_texCoord.xy + vec2(-1*scale, -2*scale);
    f_blurTexCoords[ 2] = f_texCoord.xy + vec2( 0      , -2*scale);
    f_blurTexCoords[ 3] = f_texCoord.xy + vec2( 1*scale, -2*scale);
    f_blurTexCoords[ 4] = f_texCoord.xy + vec2( 2*scale, -2*scale);

    f_blurTexCoords[ 5] = f_texCoord.xy + vec2(-2*scale, -1*scale);
    f_blurTexCoords[ 6] = f_texCoord.xy + vec2(-1*scale, -1*scale);
    f_blurTexCoords[ 7] = f_texCoord.xy + vec2( 0      , -1*scale);
    f_blurTexCoords[ 8] = f_texCoord.xy + vec2( 1*scale, -1*scale);
    f_blurTexCoords[ 9] = f_texCoord.xy + vec2( 2*scale, -1*scale);

    f_blurTexCoords[10] = f_texCoord.xy + vec2(-2*scale, 0);
    f_blurTexCoords[11] = f_texCoord.xy + vec2(-1*scale, 0);
    f_blurTexCoords[12] = f_texCoord.xy + vec2( 0      , 0);
    f_blurTexCoords[13] = f_texCoord.xy + vec2( 1*scale, 0);
    f_blurTexCoords[14] = f_texCoord.xy + vec2( 2*scale, 0);

    f_blurTexCoords[15] = f_texCoord.xy + vec2(-2*scale,  1*scale);
    f_blurTexCoords[16] = f_texCoord.xy + vec2(-1*scale,  1*scale);
    f_blurTexCoords[17] = f_texCoord.xy + vec2( 0      ,  1*scale);
    f_blurTexCoords[18] = f_texCoord.xy + vec2( 1*scale,  1*scale);
    f_blurTexCoords[19] = f_texCoord.xy + vec2( 2*scale,  1*scale);

    f_blurTexCoords[20] = f_texCoord.xy + vec2(-2*scale,  2*scale);
    f_blurTexCoords[21] = f_texCoord.xy + vec2(-1*scale,  2*scale);
    f_blurTexCoords[22] = f_texCoord.xy + vec2( 0      ,  2*scale);
    f_blurTexCoords[23] = f_texCoord.xy + vec2( 1*scale,  2*scale);
    f_blurTexCoords[24] = f_texCoord.xy + vec2( 2*scale,  2*scale);

}
