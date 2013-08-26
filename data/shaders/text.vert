#version 150

// MeshVertex attributes (see TextRenderable.cpp:initialize())
in vec3 v_position;
in vec2 v_texCoord;
in vec4 v_color;

//////////////////////////////////////////////////////////////////////
// Common Uniforms (see RenderCommand)
uniform mat4 u_projViewModelMat;     // projection * view * model
uniform mat4 u_viewModelMat;         // transforms object into camera(eye) space
uniform mat4 u_inverseViewModelMat;  // inverse of the model-view matrix, can give camera position
uniform mat4 u_projMat;              // projects camera(eye) space to clip(screen) space
uniform mat3 u_normalMat;            // transpose(inverse(viewModelMat))
uniform float u_time;                // current time (in seconds) 
uniform vec2 u_targetSizeInPixels;   // Size of the render target in pixels
//////////////////////////////////////////////////////////////////////

// Out to fragment shader
out vec4 f_fragColor;      // interpolated color of fragment from vertex colors 
out vec2 f_texCoord;       // texture coordinate of vertex
out vec4 f_vertex_screen;  // Projected vertex into the clip-space

uniform vec4 u_color = vec4( 1,1,1,1 );      // Color factor

void main()
{
	f_fragColor = v_color * u_color;
    f_texCoord = v_texCoord;
    // Scale from device pixels to unit square
    vec3 pos = vec3( v_position.x/u_targetSizeInPixels.x,
                     v_position.y/u_targetSizeInPixels.y,
                     v_position.z );
    f_vertex_screen = u_projViewModelMat * vec4( pos, 1.0 );
    gl_Position = f_vertex_screen ;
}
