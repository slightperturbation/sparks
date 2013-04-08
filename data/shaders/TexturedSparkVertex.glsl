#version 150

in vec3 position;
in vec4 inVertexColor;
in vec3 texCoord3d;

uniform mat4 modelViewMat; // projection * view * model
uniform mat4 projMat;

out vec4 FragColor; // name must match with fragment shader in-attrib
out vec2 TexCoord;  // texture coordinate of vertex
out vec4 vertexPosition;

out mat4 f_projMat;
out mat4 f_modelViewMat;
out mat4 f_modelViewProjMat;

void main()
{
    mat4 modelViewProj = projMat * modelViewMat;
    f_projMat = projMat;
    f_modelViewMat = modelViewMat;
    f_modelViewProjMat = modelViewProj;
    FragColor = inVertexColor;
    TexCoord = texCoord3d.xy;// compute texcoord from 3d coordinate  
    vertexPosition = modelViewProj * vec4( position, 1.0 );     //----- view space
    gl_Position = modelViewProj * vec4( position, 1.0 );
    //gl_Position = vec4( 0,0,0,1.0 ); 
    //gl_Position = vec4( position, 1.0 );
}

