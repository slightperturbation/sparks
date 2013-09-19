#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_vertexPosition;
in vec4 f_fragColor; // interpolated color of fragment from vertex colors
in vec3 f_texCoord;  // texture coordinate of vertex
uniform float u_time;     // time in seconds
uniform float u_activationTime; 

// For Phong Lighting
in vec4 f_normal_camera;
in vec4 f_vertex_camera;

// Samplers are named s_TEXTURENAME
uniform usampler2D s_condition;
//uniform sampler2D s_temperature;
uniform sampler2D s_color;
//uniform sampler2D s_bump;
uniform sampler2D s_normal;
//uniform sampler2D s_ambient;


///////////////////////////////////////////
// Target
uniform bool u_drawCircleTarget = true;
uniform vec2 u_targetCircleCenter = vec2( 0.6, 0.35 );
uniform float u_targetCircleOuterRadius = 0.04;
uniform float u_targetCircleInnerRadius = 0.03;
uniform vec4 u_targetCircleColor = vec4( 0, 1, 1, 1 );



// Phong-specific inputs
// struct Light
// {
// 	vec4 position_camera; // position in camera coordinates
// 	vec4 diffuse;  // diffuse color of light
// };
// uniform Light u_light;


uniform vec4 u_ambientLight = vec4( 0.2, 0.2, 0.2, 1.0 ); // ambient light color
uniform vec4 u_ks = vec4( 1, 1, 1, 1 ); // material specular coefficient, typically white
uniform float u_ns = 50.1; // material specular exponent, ~50 is a good value for sharp highlights


/// Lights hard-coded
// light position in view-space (aka camera)
vec3 u_lightPos = normalize( vec3( 0.0, 1.0, 0.0 ) );
vec4 u_lightDiffuse = vec4( 1,1,1,1 );


float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float circle( vec2 position, vec2 center, float radius ) 
{
	return 1.0 - smoothstep( radius*0.8, radius, length(position-center) );
}

void main()
{
    //vec4 temp = texture( s_temperature, f_texCoord.xy );
    //float t = (temp.r - 37.0)/63.0;
    vec2 texCoord = f_texCoord.xy * vec2(3.3,3.3); 

    vec3 liverColor = texture( s_color, texCoord).xyz; 
    //vec3 liverColor = vec3( 0.6, 0.1, 0.15 );
    vec3 normal = normalize( (texture( s_normal, texCoord).rgb * 2.0 - 1.0) ); // re-map to [-1, 1]

    vec3 dessicatedColor = vec3( 0.9, 0.9, 0.9 ) + vec3(0.1) * rand(f_vertexPosition.xz);
    vec3 vaporizingColor = vec3( 0.9, 0.5, 0.3 ) + vec3(0.2) * rand(f_vertexPosition.xz * u_time);
    vec3 charredColor = vec3( 0.1, 0.025, 0.01 ) + vec3(0.2) * rand(f_vertexPosition.xz); 

    vec3 tissueColor = vec3(0,0,0);
    // limit to five lookups when possible
    uint centerCond = texture( s_condition, f_texCoord.xy ).r;
	tissueColor += float(centerCond==0u) * liverColor;
	tissueColor += float(centerCond==1u) * dessicatedColor;
	tissueColor += float(centerCond==2u) * vaporizingColor;
	tissueColor += float(centerCond==3u) * charredColor;
	vec4 textureColor = vec4( tissueColor, 1 );
	/////////////////////////////////////////////

	vec4 normal_camera = vec4( normal, 0 ); // normal direction
	vec4 toLight_camera = normalize( vec4(u_lightPos,1) - f_vertex_camera );
	vec4 toCamera_camera = normalize( -f_vertex_camera );
	vec4 halfVector_camera = normalize( toCamera_camera + toLight_camera ); // Blinn's half-vector
	//vec4 reflect_camera = reflect( -toLight_camera, toCamera_camera ); // Phong's reflection model

	if( u_drawCircleTarget )
	{    
		textureColor +=  vec4( 0.8, 0.8, 0.4, 1 ) 
			* ( circle(  f_texCoord.xy, u_targetCircleCenter, u_targetCircleOuterRadius )
			   - circle( f_texCoord.xy, u_targetCircleCenter, u_targetCircleInnerRadius )
			);
	}
    
	vec4 Ia = u_ambientLight * textureColor;
	vec4 Id = u_lightDiffuse * textureColor * max( dot (toLight_camera, normal_camera), 0.4 );
	vec4 Is = vec4( 1,1,1,1 ) * u_ks * pow( max(dot(halfVector_camera, normal_camera), 0.1), u_ns );
    
	outColor = Ia + Id + Is;
}