#version 150
//shadowCaster.frag
 
// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;


void main()
{
	outColor = vec4(gl_FragCoord.z);
}


