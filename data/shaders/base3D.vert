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
out vec3 f_texCoord;       // texture coordinate of vertex
out vec4 f_vertex_screen;  // Projected vertex into the clip-space
out vec4 f_normal_camera;  // For phong lighting
out vec4 f_vertex_camera;  // For phong lighting

out vec3 f_blurTexCoords[6];
uniform float u_blurRadius = 1.0/64.0; // depends on the # of slices

void main()
{
    f_normal_camera = vec4( u_normalMat * v_normal, 0.0 ); // dir
    f_vertex_camera = u_viewModelMat * vec4( v_position, 1.0 ); // point

    // TEMP-- testing with white fragment color output
    f_fragColor = vec4( 1,1,1,1 );//v_color ;
    f_texCoord = v_texCoord;  
    f_vertex_screen = u_projViewModelMat * vec4( v_position, 1.0 );
    gl_Position = f_vertex_screen;

    float s = u_blurRadius;
    f_blurTexCoords[0] = f_texCoord + vec3( -1*s,    0,    0 );
    f_blurTexCoords[1] = f_texCoord + vec3( +1*s,    0,    0 );
    f_blurTexCoords[2] = f_texCoord + vec3(    0, -1*s,    0 );
    f_blurTexCoords[3] = f_texCoord + vec3(    0, +1*s,    0 );
    f_blurTexCoords[4] = f_texCoord + vec3(    0,    0, -1*s );
    f_blurTexCoords[5] = f_texCoord + vec3(    0,    0, +1*s );
}
