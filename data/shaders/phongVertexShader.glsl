#version 150

// MeshVertex attributes (see Mesh.hpp)
in vec3 v_position;
in vec3 v_normal;
in vec4 v_color;
in vec3 v_texCoord;

// Perspective uniforms (see Matierial::setTransformUniforms)
uniform mat4 u_projViewModelMat; // projection * view * model
uniform mat4 u_viewModelMat;     // transforms object into camera(eye) space
uniform mat4 u_projMat;          // projects camera(eye) space to clip(screen) space

// Non-standard uniforms
uniform vec4 u_color;

// Out to fragment shader
out vec4 f_fragColor; // interpolated color of fragment from vertex colors 
out vec2 f_texCoord;  // texture coordinate of vertex
out vec4 f_vertexPosition;

void main()
{
    f_fragColor = v_color ;// * u_color;
    f_texCoord = v_texCoord.st;// take 2d texcoord from 3d coordinate  
    f_vertexPosition = u_projViewModelMat * vec4( v_position, 1.0 );
    gl_Position = f_vertexPosition;
}
