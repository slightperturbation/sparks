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
//////////////////////////////////////////////////////////////////////

// Out to fragment shader
out vec4 f_fragColor;      // interpolated color of fragment from vertex colors 
out vec2 f_texCoord;       // texture coordinate of vertex
out vec4 f_vertex_screen;  // Projected vertex into the clip-space
out vec4 f_normal_camera;  // For phong lighting
out vec4 f_vertex_camera;  // For phong lighting
out vec3 f_normal;
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
out vec2 f_blurTexCoords[9];
// Blur radius == this should be tied directly to the 
// size of the tissue condition map!
// 1/map_size
uniform float u_blurRadius = 1.0/512.0;

// Allow scaling factor for the depthMap
uniform float u_depthScale = 1.0;
uniform sampler2D s_depthMap;

void main()
{
    f_texCoord = u_textureRepeat * v_texCoord.st;// take 2d texcoord from 3d coordinate  

    // Smooth depth
	float depth = texture( s_depthMap, f_texCoord ).r;

	vec4 mappedPosition = vec4( v_position, 1 );
	mappedPosition.y -= depth * u_depthScale;
    //mappedPosition.y += 0.1 * sin( f_texCoord.s + f_texCoord.t );
    f_vertex_screen = u_projViewModelMat * mappedPosition;

    f_vertexPosition = mappedPosition;
	f_normal_camera = vec4( u_normalMat * v_normal, 0.0 ); // dir
	f_vertex_camera = u_viewModelMat * mappedPosition; // point
    f_fragColor = v_color ;

    f_vertex_screen = u_projViewModelMat * mappedPosition;
    f_shadowPosition = u_shadowLight[u_currLightIndex].projViewModelMat * mappedPosition;
    gl_Position = f_vertex_screen;


    float scale = u_blurRadius;
    f_blurTexCoords[ 0] = f_texCoord.xy + vec2(-1*scale, -1*scale);
    f_blurTexCoords[ 1] = f_texCoord.xy + vec2( 0      , -1*scale);
    f_blurTexCoords[ 2] = f_texCoord.xy + vec2( 1*scale, -1*scale);

    f_blurTexCoords[ 3] = f_texCoord.xy + vec2(-1*scale, 0);
    f_blurTexCoords[ 4] = f_texCoord.xy + vec2( 0      , 0);
    f_blurTexCoords[ 5] = f_texCoord.xy + vec2( 1*scale, 0);

    f_blurTexCoords[ 6] = f_texCoord.xy + vec2(-1*scale,  1*scale);
    f_blurTexCoords[ 7] = f_texCoord.xy + vec2( 0      ,  1*scale);
    f_blurTexCoords[ 8] = f_texCoord.xy + vec2( 1*scale,  1*scale);

}
