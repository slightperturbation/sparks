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

uniform vec2 u_textureRepeat = vec2(1,1);

struct ShadowLight 
{
    mat4 projViewModelMat;
    vec4 color;
};

uniform ShadowLight u_shadowLight[4];
uniform int u_currLightIndex = 0;

void main()
{
	// gl_Position = u_shadowLight[u_currLightIndex].projViewModelMat
	//               * vec4( v_position, 1.0 );
	gl_Position = //u_shadowLight[u_currLightIndex].projViewModelMat
	              u_projViewModelMat * vec4( v_position, 1.0 );
}
