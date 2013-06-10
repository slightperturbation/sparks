#version 150

//////////////////////////////////////////////////////////////////////
// Standard output.  See ShaderManager::reloadShader()
out vec4 outColor;
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Common Uniforms (see RenderCommand)
uniform mat4 u_projViewModelMat;     // projection * view * model
uniform mat4 u_viewModelMat;         // transforms object into camera(eye) space
uniform mat4 u_inverseViewModelMat;  // inverse of the model-view matrix, can give camera position
uniform mat4 u_projMat;              // projects camera(eye) space to clip(screen) space
uniform mat3 u_normalMat;            // transpose(inverse(viewModelMat))
uniform float u_time;                // current time (in seconds) 
//////////////////////////////////////////////////////////////////////

// From vertex shader
in vec4 f_fragColor;                 // interpolated color of fragment from vertex colors 
in vec3 f_texCoord;                  // texture coordinate of vertex
in vec4 f_vertex_screen;             // projected vertex position
in vec4 f_normal_camera;             // For phong lighting
in vec4 f_vertex_camera;             // unprojected vertex position

uniform sampler3D s_density3d;       // Density 3d texture data

// Shader-specific uniforms
uniform vec3 u_lightPosition_world;  
uniform vec3 u_lightColor;
uniform float u_absorption = 0.3;          // Fraction of light abosorbed by 
uniform int u_numSamples = 64; 
uniform int u_numLightSamples = 128;

//uniform float g_time; // for seeding the random number generator

/// psuedo random number generator from http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
float rand(vec2 seed2d)
{
    return fract(sin(dot(seed2d.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
	// based on http://blog.mmacklin.com/2010/11/01/adventures-in-fluid-simulation/
	const float maxDist = sqrt( 3.0 ) + 0.01;
	int numSamples = u_numSamples; //64;
	int numLightSamples = u_numLightSamples; //128;
	const float minDensity = 0.01;
	const float randomScale = 0.05;

	float scale = maxDist / float(numSamples);
	float lscale = maxDist / float(numLightSamples);  // todo - need to compute the distance from lpos to edge... 
	
	// Eye position in voxel coordinates is the inverse of the model-view translation
	// todo-- don't need a real inverse here because this is an rigid transform, 
	// ie:  M = TR, M^(-1) = (TR)^(-1) = (R^T)(-T)
	mat4 invModelView = inverse(u_viewModelMat);
	//mat4 invModelView = transpose(mat3(f_modelViewMat)) * (mat3(1.0);
	vec3 eyePos = (invModelView * vec4(0,0,0,1)).xyz;//invModelView[3].xyz works too, but this is more obvious?
	
	vec3 pos = f_texCoord; // position in voxel coordinates
	// a vector in the direction of the eye, with length of one "step" toward it
	vec3 eyeDir = normalize( pos - eyePos ) * scale;

	// accumulated transmitance
	float T = 1.0; 
	// in-scattered radiance
	vec3 Lo = vec3( 0.0 );
	vec2 randSeed = vec2( 0.0, 1.0 );
	for( int i = 0; i < numSamples; ++i )
	{
		// get the density at this position
		float density = texture( s_density3d, pos ).r;
		randSeed.x += density * numSamples;
		randSeed.y *= density + 1.0;
		// ignore empty space
		if( density > minDensity )
		{
			// attenuate ray for passing through this chunk of density
			//T -= density*scale*absorption;
			T *= 1.0 - (density * scale * u_absorption);

			// a vector in the direction of the light, with length of one "step" toward it
			vec3 lightDir = normalize( u_lightPosition_world - pos ) * lscale;

			// accumulated transmittance along light ray
			float Tl = 1.0;
			vec3 lpos = pos + lightDir;

			for( int s = 0; s < numLightSamples; ++s )
			{
				if( dot( (u_lightPosition_world - lpos), lightDir ) < 0 )
				{
					break; // stepped past the light
				} 
				float ld = texture( s_density3d, lpos ).r; // density at this light sample
				Tl *= 1.0 - ( ld * lscale * u_absorption );
				if( Tl <= 0.01 )
				{
					break;
				}
				lpos += lightDir * ( 1.0 + randomScale * (0.5 - rand(randSeed)) );
			}
			// have the illumination level of the light in Tl
			// accumlate the effect of this sample
			//Lo += ((texture( s_density3d, pos ).rgb * f) + (u_lightColor * (1.0-f))) * Tl * T * density * scale ;//* T * density * scale;
			Lo += u_lightColor * Tl * T * density * scale ;//* T * density * scale;

			if( T <= 0.01 )
			{
				break; // can't see anymore, transmittence is low, stop sampling
			}
		}
		// advance the sampling point toward the light
		pos += eyeDir * ( 1.0 + randomScale * (0.5 - rand(randSeed)) );
	}

	// final color for this sample is the lighted color, with total transmittance
	outColor = vec4( Lo, 1.0 - T );
}
