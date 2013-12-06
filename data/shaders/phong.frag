#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_vertexPosition;
in vec4 f_fragColor; // interpolated color of fragment from vertex colors 
in vec2 f_texCoord;  // texture coordinate of vertex
in float f_time;     // time in seconds

// For Phong Lighting
in vec4 f_normal_camera;
in vec4 f_vertex_camera;
in vec3 f_normal;

// For Shadows
in vec4 f_shadowPosition;       // position of fragemnt in shadow's coordinate frame
uniform sampler2D s_shadowMap;

// Base color texture, samplers are named s_TEXTURENAME
uniform sampler2D s_color;  // diffuse texture
uniform sampler2D s_normal; // normal map

uniform float u_normalMapStrength = 0.5;

// Phong-specific inputs
struct Light
{
	vec4 position_camera; // position in camera coordinates
	vec4 diffuse;  // diffuse color of light
};
uniform Light u_light;
uniform vec4 u_ambientLight = vec4( 0.2, 0.2, 0.2, 1.0 ); // ambient light
uniform vec4 u_ks = vec4( 1, 1, 1, 1 ); // material specular coefficient, typically white
uniform float u_ns = 15; // material specular exponent, ~15-20 is a good value for sharp highlights


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
		factor = 0.2; // in shade factor
	}
	return factor;
}

void main()
{
	const float cutoff = 0.1;


    //vec3 normal = normalize( (texture( s_normal, f_texCoord).rgb * 2.0 - 1.0) ); // re-map to [-1, 1]

	// the coordinate system follows the subscript (camera == view)
	vec4 normal_camera = normalize( f_normal_camera );
    normal_camera = normalize( normal_camera + u_normalMapStrength*vec4(texture( s_normal, f_texCoord).rgb * 2.0 - 1.0, 0.0) ); // re-map to [-1, 1]
	vec4 toLight_camera = normalize( u_light.position_camera - f_vertex_camera );
	vec4 toCamera_camera = normalize( -f_vertex_camera ); // camera is at origin in camera space
	//vec4 halfVector_camera = normalize( toCamera_camera + toLight_camera ); // Blinn's half-vector
	vec4 reflect_camera = reflect( -toLight_camera, toCamera_camera ); // Phong's reflection model

	vec4 textureColor = texture( s_color, f_texCoord ); 
	//vec4 textureColor = vec4(1.0);
	if( textureColor.a < cutoff )
	{
		discard;
	}

	vec4 Ia = u_ambientLight * textureColor;
	vec4 Id = u_light.diffuse * textureColor * max( dot (toLight_camera, normal_camera), 0.0 );
	//vec4 Is = vec4( 1,1,1,1 ) * u_ks * pow( max(dot(halfVector_camera, normal_camera), 0.0), u_ns );
	vec4 Is = vec4( 1,1,1,1 ) * u_ks * pow( max(dot(reflect_camera, normal_camera), 0.0), u_ns );

	outColor = Ia + (Id + Is) * shadowFactor();

	//outColor = u_light.diffuse * textureColor * max( dot (toLight_camera, normal_camera), 0.1 );
	//show normals as color (-1,1)->(0,1)
	//outColor = vec4( 0.5 * f_normal + vec3(.5), 1 );
	//outColor = 0.5 * normal_camera + vec4(0.5);
	//outColor = vec4( 0, 0, 1, 1 );
}
