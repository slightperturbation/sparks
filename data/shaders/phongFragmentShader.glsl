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

// Samplers are named s_TEXTURENAME
uniform sampler2D s_color;
uniform sampler2D s_color2;

// Phong-specific inputs
struct Light
{
	vec4 position_camera; // position in camera coordinates
	vec4 diffuse;  // diffuse color of light
};
uniform Light u_light;
uniform vec4 u_ambientLight; // ambient light
uniform vec4 u_ks; // material specular coefficient, typically white
//uniform float u_ns; // material specular exponent, ~50 is a good value for sharp highlights

void main()
{
	const float cutoff = 0.1;
	// Overrides
	float u_ns = 50.1;

	vec4 normal_camera = normalize( f_normal_camera );
	vec4 toLight_camera = normalize( u_light.position_camera - f_vertex_camera );
	vec4 toCamera_camera = normalize( -f_vertex_camera );
	vec4 halfVector_camera = normalize( toCamera_camera + toLight_camera ); // Blinn's half-vector
	//vec4 reflect_camera = reflect( -toLight_camera, toCamera_camera ); // Phong's reflection model

	vec4 textureColor = texture( s_color, f_texCoord );
	if( textureColor.a < cutoff )
	{
		discard;
	}

	vec4 Ia = u_ambientLight * textureColor;
	vec4 Id = u_light.diffuse * textureColor * max( dot (toLight_camera, normal_camera), 0.0 );
	vec4 Is = vec4( 1,1,1,1 ) * u_ks * pow( max(dot(halfVector_camera, normal_camera), 0.0), u_ns );

	outColor = Ia + Id + Is;
}