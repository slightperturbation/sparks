#version 150

// MeshVertex attributes (see Mesh.hpp)
in vec3 v_position;
in vec3 v_normal;
in vec4 v_color;
in vec2 v_texCoord;

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

uniform vec4 u_color = vec4( 1,1,1,1 );      // Color factor

void main()
{
	f_normal_camera = vec4( u_normalMat * v_normal, 0.0 ); // dir
	f_vertex_camera = u_viewModelMat * vec4( v_position, 1.0 ); // point
    f_fragColor = v_color * u_color;
    f_texCoord = v_texCoord;  
    f_vertex_screen = u_projViewModelMat * vec4( v_position, 1.0 );
    gl_Position = f_vertex_screen;
}