#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_fragColor;      // interpolated color of fragment from vertex colors 
in vec2 f_texCoord;       // texture coordinate of vertex
in vec4 f_vertex_screen;  // Projected vertex into the clip-space

uniform sampler2D s_color;

vec4 blendFunc( vec4 c1, vec4 c2 )
{
	vec4 o = c1.a*c1 + c2.a+c2;
	return o;
}

void main(void)
{
    vec4  color = texture( s_color, f_texCoord );
//    float dist  = color.r;
    float dist  = color.r;
    float width = fwidth( dist );
    float alpha = smoothstep( 0.5 - width, 0.5 + width, dist );
    outColor = vec4( f_fragColor.rgb, alpha * f_fragColor.a ) ;

    // border
    // vec4 borderColor = vec4(0,0,0,0);
    // float borderWidth = 0.001;
    // float borderDist = f_texCoord.x*f_texCoord.x + 0.00001;
    // outColor += borderColor * clamp( 1 - borderDist/borderWidth, 0, 1 );

}

