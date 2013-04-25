#version 150

// MeshVertex attributes (see Mesh.hpp)
in vec3 position;
in vec3 normal;
in vec4 inVertexColor;
in vec3 texCoord3d;

// Perspective uniforms (see Matierial.hpp)
uniform mat4 u_modelViewMat;
uniform mat4 u_modelViewProjMat; // projection * view * model
uniform mat4 u_projMat;

// Out to fragment shader
out vec4 f_fragColor; // interpolated color of fragment from vertex colors 
out vec3 f_texCoord;  // texture coordinate of vertex
out vec4 f_vertexPosition;

void main()
{
    f_fragColor = inVertexColor;
    f_texCoord = texCoord3d;// compute texcoord from 3d coordinate  
    f_vertexPosition = u_modelViewProjMat * vec4( position, 1.0 );
    gl_Position = u_modelViewProjMat * vec4( position, 1.0 );
}
