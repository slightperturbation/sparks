#version 150

// MeshVertex attributes (see Mesh.hpp)
in vec3 v_position;
in vec3 v_normal;
in vec4 v_color;
in vec3 v_texCoord;

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

// Shader parameters
uniform float u_blurRadius = 0.05; // Fraction of target blur will extend to

// Out to fragment shader
out vec4 f_fragColor;      // interpolated color of fragment from vertex colors 
out vec2 f_texCoord;       // texture coordinate of vertex
out vec4 f_vertex_screen;  // Projected vertex into the clip-space
out vec4 f_normal_camera;  // For phong lighting
out vec4 f_vertex_camera;  // For phong lighting

out vec2 f_blurTexCoords[14];

void main()
{
	f_normal_camera = vec4( u_normalMat * v_normal, 0.0 ); // dir
	f_vertex_camera = u_viewModelMat * vec4( v_position, 1.0 ); // point
    f_fragColor = v_color ;
    f_texCoord = vec2( v_texCoord.s, 1 - v_texCoord.t );// flip tex coord to orient vs render
    f_vertex_screen = u_projViewModelMat * vec4( v_position, 1.0 );

    float scale = 0.142857143 * u_blurRadius;
    f_blurTexCoords[ 0] = f_texCoord + vec2(0.0, -7*scale);
    f_blurTexCoords[ 1] = f_texCoord + vec2(0.0, -6*scale);
    f_blurTexCoords[ 2] = f_texCoord + vec2(0.0, -5*scale);
    f_blurTexCoords[ 3] = f_texCoord + vec2(0.0, -4*scale);
    f_blurTexCoords[ 4] = f_texCoord + vec2(0.0, -3*scale);
    f_blurTexCoords[ 5] = f_texCoord + vec2(0.0, -2*scale);
    f_blurTexCoords[ 6] = f_texCoord + vec2(0.0, -1*scale);
    f_blurTexCoords[ 7] = f_texCoord + vec2(0.0,  1*scale);
    f_blurTexCoords[ 8] = f_texCoord + vec2(0.0,  2*scale);
    f_blurTexCoords[ 9] = f_texCoord + vec2(0.0,  3*scale);
    f_blurTexCoords[10] = f_texCoord + vec2(0.0,  4*scale);
    f_blurTexCoords[11] = f_texCoord + vec2(0.0,  5*scale);
    f_blurTexCoords[12] = f_texCoord + vec2(0.0,  6*scale);
    f_blurTexCoords[13] = f_texCoord + vec2(0.0,  7*scale);

    gl_Position = f_vertex_screen;
}
