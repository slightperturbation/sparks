#version 150

// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;

// From vertex shader
in vec4 f_fragColor; // interpolated color of fragment from vertex colors 
in vec2 f_texCoord;  // texture coordinate of vertex
in vec4 f_vertexPosition;
in vec3 f_normal;
in vec4 f_normal_camera;  		// Normal direction
in vec4 f_shadowPosition;       // position of fragemnt in shadow's coordinate frame


//////////////////////////////////////////////////////////////////////
// Common Uniforms (see RenderCommand)
uniform mat4 u_projViewModelMat;     // projection * view * model
uniform mat4 u_viewModelMat;         // transforms object into camera(eye) space
uniform mat4 u_inverseViewModelMat;  // inverse of the model-view matrix, can give camera position
uniform mat4 u_projMat;              // projects camera(eye) space to clip(screen) space
uniform mat3 u_normalMat;            // transpose(inverse(viewModelMat))
uniform float u_time;                // current time (in seconds) 
//////////////////////////////////////////////////////////////////////

uniform vec4 u_color = vec4( 1, 1, 1, 1 );

struct ShadowLight 
{
    mat4 projViewModelMat;
    mat4 biasProjViewModelMat;
    vec4 color;
};
uniform ShadowLight u_shadowLight[4];
uniform int u_currLightIndex = 0;

uniform sampler2D s_shadowMap;

// Returns a random number based on a vec3 and an int.
// From: http://www.opengl-tutorial.org/
float random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

void main()
{

    vec4 shadowPos = f_shadowPosition;
	shadowPos /= shadowPos.w;  // projection divide 
    // Switch from homogeneous coords (-1,1) to texture coords (0,1)
    shadowPos += 1.0; 
    shadowPos *= 0.5;

	outColor = vec4( 1, 1, 1, 1 );
	/// Read from the shadow map
    float distFragToLight = texture( s_shadowMap, shadowPos.xy ).r;

	int secs = 3;//int(u_time);
	int versions = 6;
	if( 0 == (secs % versions) )
	{
		// dist of nearst geo to light (shadowmap)
		outColor = vec4( vec3(distFragToLight), 1 );
	}
	if( 1 == (secs % versions) )
	{
		// dist of frag to light
		outColor = vec4( vec3( shadowPos.z ), 1 );
	}
	if( 2 == (secs % versions) )
	{
		// No bias
		if (  distFragToLight < (shadowPos.z) )
		{
		    outColor = vec4( 0.1, 0.1, 0.1, 1 );
		}
		else
		{
			outColor = vec4( 0.4, 0, 0, 1 );
		}
	}
	if( 3 == (secs % versions) )
	{
		// With fixed, bad guess bias
	    float bias = 0.0015; 
		if (  distFragToLight < (shadowPos.z - bias) )
		{
		    outColor = vec4( 0.1, 0.1, 0.1, 1 );
		}
		else
		{
			outColor = vec4( 1,1,1, 1 );
		}
	}
	if( 4 == (secs % versions) )
	{
		// BROKEN -- need light pos/dir and vector in same coord frame.  What is f_normal in?
		vec4 lightDir_world = vec4( 0.5, 2, 2, 1 ); // TODO---- need to add direction/position to Light struct

		float cosTheta = dot( f_normal_camera, lightDir_world );
		float bias = 0.005*tan( acos( cosTheta ) ); // cosTheta is dot( n,l ), clamped between 0 and 1
		bias = clamp( bias, 0, 0.01 );

		if( (distFragToLight < (shadowPos.z - bias)) && (shadowPos.w > 0.0) )
		{
		    outColor = vec4( 0, 0, 0.2, 1 );
		}
		else
		{
			outColor = vec4( 0.5,1,1, 1 );
		}
	}
	if( 5 == (secs % versions ) )
	{
		vec2 poissonDisk[16] = vec2[]( 
		   vec2( -0.94201624, -0.39906216 ), 
		   vec2( 0.94558609, -0.76890725 ), 
		   vec2( -0.094184101, -0.92938870 ), 
		   vec2( 0.34495938, 0.29387760 ), 
		   vec2( -0.91588581, 0.45771432 ), 
		   vec2( -0.81544232, -0.87912464 ), 
		   vec2( -0.38277543, 0.27676845 ), 
		   vec2( 0.97484398, 0.75648379 ), 
		   vec2( 0.44323325, -0.97511554 ), 
		   vec2( 0.53742981, -0.47373420 ), 
		   vec2( -0.26496911, -0.41893023 ), 
		   vec2( 0.79197514, 0.19090188 ), 
		   vec2( -0.24188840, 0.99706507 ), 
		   vec2( -0.81409955, 0.91437590 ), 
		   vec2( 0.19984126, 0.78641367 ), 
		   vec2( 0.14383161, -0.14100790 ) 
		);

	    float bias = 0.0015; 
		
		float visibility = 1;
    	float distFragToLight = texture( s_shadowMap, shadowPos.xy ).r;
    	if( shadowPos.w > 0.0 )
    	{
		    for( int i = 0; i < 4; i++ )
		    {
		    	int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
		    	//int index = int( 16.0 * random( floor(f_vertexPosition.xyz * 10000.0), i ) ) % 16;
				vec2 jitteredPos = shadowPos.xy + poissonDisk[index] / 1500.0;
				float distFragToLightJittered = texture( s_shadowMap, jitteredPos ).r;
				if (  distFragToLightJittered < (shadowPos.z - bias) )
				{
					visibility -= 0.2; // total shadow is 0.2 = (1 - 4*0.2)
				}	

				// int index = i; // A random number between 0 and 15, different for each pixel (and each i !)
				// visibility -= 0.2*(1.0-texture( s_shadowMap, vec3(shadowPos.xy + poissonDisk[index]/700.0, shadowPos.z-bias) ));
		    }
    	}
	    outColor = vec4( 1, 1, 1, 1 );
	    outColor *= visibility;
	}
}
