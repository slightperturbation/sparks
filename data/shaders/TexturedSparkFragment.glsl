#version 150

out vec4 outColor;
in vec4 FragColor; // name must match with vertex shader out-attrib
in vec2 TexCoord;  // name must match with vertex shader out-attrib
in vec4 vertexPosition;
in mat4 ModelViewProj;
uniform sampler2D tex2d;
//uniform sampler3D tex3d;

void main()
{
	//float cutoff = 0.5;
	//if( outColor.a > cutoff )
	//{
	// 	discard;
	//}
	outColor = texture( tex2d, TexCoord );// * FragColor;

	//outColor = vec4( TexCoord.t, 0.0, 1.0, 1.0 );
	//outColor.rgb *= smoothstep( cutoff, 1.0, outColor.a );
    //outColor = vec4( FragColor, 0.5 ) * texture( tex2d, TexCoord );
    //outColor = vec4( FragColor.xyz, 0.5 );
    //outColor = FragColor;
    //if( TexCoord.x < 0.1 || TexCoord.x > 0.9 )
    
    	//outColor =  FragColor;
    
}