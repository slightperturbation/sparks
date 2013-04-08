#version 150

out vec4 outColor;
in vec3 TexCoord;  // name must match with vertex shader out-attrib

in vec4 vertexPosition; //-- view space

in mat4 f_projMat;
in mat4 f_modelViewMat;
in mat4 f_modelViewProjMat;

uniform vec3 g_worldLightPosition;

uniform sampler3D fluidData3d;

uniform float scalarTextureEnabled;
uniform float vectorTextureEnabled;

uniform vec3 g_lightColor;
uniform float g_absorption;

uniform int g_numSamples;
uniform int g_numLightSamples;

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
	int numSamples = g_numSamples; //64;
	int numLightSamples = g_numLightSamples; //128;
	const float minDensity = 0.01;
	const float randomScale = 0.05;

	if( scalarTextureEnabled == 0.0 )
	{
		outColor = vec4( 1,1,1,1 );
		return;
	}

	float scale = maxDist / float(numSamples);
	float lscale = maxDist / float(numLightSamples);  // todo - need to compute the distance from lpos to edge... 
	
	// Eye position in voxel coordinates is the inverse of the model-view translation
	// todo-- don't need a real inverse here because this is an rigid transform, 
	// ie:  M = TR, M^(-1) = (TR)^(-1) = (R^T)(-T)
	mat4 invModelView = inverse(f_modelViewMat);
	//mat4 invModelView = transpose(mat3(f_modelViewMat)) * (mat3(1.0);
	vec3 g_eyePos = (invModelView * vec4(0,0,0,1)).xyz;//invModelView[3].xyz works too, but this is more obvious?
	
	vec3 pos = TexCoord; // position in voxel coordinates

	// a vector in the direction of the eye, with length of one "step" toward it
	vec3 eyeDir = normalize( pos - g_eyePos ) * scale;

	// Use coloring from data or lighting -- allornothing for now.
	float f = 0.0;
	if( vectorTextureEnabled == 1.0 )
	{
		f = 1.0;
	}

	// accumulated transmitance
	float T = 1.0; 
	// in-scattered radiance
	vec3 Lo = vec3( 0.0 );
	vec2 randSeed = vec2( 0.0, 1.0 );
	for( int i = 0; i < numSamples; ++i )
	{
		// get the density at this position
		float density = texture( fluidData3d, pos ).a;
		randSeed.x += density * numSamples;
		randSeed.y *= density + 1.0;
		// ignore empty space
		if( density > minDensity )
		{
			// attenuate ray for passing through this chunk of density
			//T -= density*scale*absorption;
			T *= 1.0 - (density * scale * g_absorption);

			// a vector in the direction of the light, with length of one "step" toward it
			vec3 lightDir = normalize( g_worldLightPosition - pos ) * lscale;

			// accumulated transmittance along light ray
			float Tl = 1.0;
			vec3 lpos = pos + lightDir;

			for( int s = 0; s < numLightSamples; ++s )
			{
				if( dot( (g_worldLightPosition - lpos), lightDir ) < 0 )
				{
					break; // stepped past the light
				} 
				float ld = texture( fluidData3d, lpos ).a; // density at this light sample
				Tl *= 1.0 - ( ld * lscale * g_absorption );
				if( Tl <= 0.01 )
				{
					break;
				}
				lpos += lightDir * ( 1.0 + randomScale * (0.5 - rand(randSeed)) );
			}
			// have the illumination level of the light in Tl
			// accumlate the effect of this sample
			Lo += ((texture( fluidData3d, pos ).rgb * f) + (g_lightColor * (1.0-f))) * Tl * T * density * scale ;//* T * density * scale;

			if( T <= 0.01 )
			{
				break; // can't see anymore, transmittence is low, stop sampling
			}
		}
		// advance the sampling point toward the light
		pos += eyeDir * ( 1.0 + randomScale * (0.5 - rand(randSeed)) );
	}

	// final color for this sample is the lighted color, with total transmittance
	outColor = vec4( Lo, 1.0 - T );//vec4( Lo, 1.0 - T );
}
