#version 150
// tissue_bump.frag

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_vertexPosition;
in vec4 f_fragColor; // interpolated color of fragment from vertex colors
in vec3 f_texCoord;  // texture coordinate of vertex
uniform float u_time;     // time in seconds
uniform float u_activationTime; 
in vec4 f_shadowPosition;       // position of fragemnt in shadow's coordinate frame

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


// Blur samples
in vec2 f_blurTexCoords[25];

struct ShadowLight 
{
    mat4 projViewModelMat;
    mat4 biasProjViewModelMat;
    vec4 color;
};
uniform ShadowLight u_shadowLight[4];
uniform int u_currLightIndex = 0;

uniform sampler2D s_shadowMap;

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

/// Returns the amount of light the current pixel is in, between 0.2 and 1.0
float shadowFactor()
{
	vec4 shadowPos = f_shadowPosition;
	shadowPos /= shadowPos.w;  // projection divide 
    // Switch from homogeneous coords (-1,1) to texture coords (0,1)
    shadowPos += 1.0; 
    shadowPos *= 0.5;
	/// Read from the shadow map
    float distFragToLight = texture( s_shadowMap, shadowPos.xy ).r;
    float bias = 0.0015;
    float factor = 1; 
	if (  distFragToLight < (shadowPos.z - bias) )
	{
		factor = 0.2;
	}
	return factor;
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

    float blurWeight[25] = float[]( 1.0/273.0, 4.0/273.0, 7.0/273.0, 4.0/273.0, 1.0/273.0,
    	                            4.0/273.0,16.0/273.0,26.0/273.0,16.0/273.0, 4.0/273.0,
    	                            7.0/273.0,26.0/273.0,41.0/273.0,26.0/273.0, 7.0/273.0,
    	                            4.0/273.0,16.0/273.0,26.0/273.0,16.0/273.0, 4.0/273.0,
    	                            1.0/273.0, 4.0/273.0, 7.0/273.0, 4.0/273.0, 1.0/273.0 );

    vec3 tissueColor = vec3(0,0,0);
    // limit to five lookups when possible
    uint centerCond = texture( s_condition, f_blurTexCoords[12].xy ).r;
    uint condCorner0 = texture( s_condition, f_blurTexCoords[0].xy ).r;
    uint condCorner4 = texture( s_condition, f_blurTexCoords[4].xy ).r;
    uint condCorner21 = texture( s_condition, f_blurTexCoords[21].xy ).r;
    uint condCorner24 = texture( s_condition, f_blurTexCoords[24].xy ).r;
    if(     centerCond == condCorner0
         && centerCond == condCorner4
         && centerCond == condCorner21
         && centerCond == condCorner24 )
    {
		tissueColor += float(centerCond==0u) * liverColor;
		tissueColor += float(centerCond==1u) * dessicatedColor;
		tissueColor += float(centerCond==2u) * vaporizingColor;
		tissueColor += float(centerCond==3u) * charredColor;
    }
    else
    {
	    for( int i = 0; i < 25; ++i )
	    {
		    uint cond = texture( s_condition, f_blurTexCoords[i].xy ).r;
			
		    /// normalTissue=0, dessicatedTissue=1, vaporizingTissue=2, charredTissue=3.
			tissueColor += float(cond==0u) * blurWeight[i] * liverColor;
			tissueColor += float(cond==1u) * blurWeight[i] * dessicatedColor;
			tissueColor += float(cond==2u) * blurWeight[i] * vaporizingColor;
			tissueColor += float(cond==3u) * blurWeight[i] * charredColor;
		}
	}
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
	vec4 Id = shadowFactor() * u_lightDiffuse * textureColor * max( dot (toLight_camera, normal_camera), 0.4 );
	vec4 Is = shadowFactor() * vec4( 1,1,1,1 ) * u_ks * pow( max(dot(halfVector_camera, normal_camera), 0.1), u_ns );
    
	outColor = Ia + Id + Is;
}