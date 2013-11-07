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

// Blur samples
in vec2 f_blurTexCoords[25];

// Phong-specific inputs
struct Light
{
	vec4 position_camera; // position in camera coordinates
	vec4 diffuse;  // diffuse color of light
};
uniform Light u_light;
uniform vec4 u_ambientLight = vec4( 0.2, 0.2, 0.2, 1.0 ); // ambient light
uniform vec4 u_ks = vec4( 1, 1, 1, 1 ); // material specular coefficient, typically white
uniform float u_ns = 50.1; // material specular exponent, ~50 is a good value for sharp highlights




// Cellular noise ("Worley noise") in 3D in GLSL.
// Copyright (c) Stefan Gustavson 2011-04-19. All rights reserved.
// This code is released under the conditions of the MIT license.
// See LICENSE file for details.

// Permutation polynomial: (34x^2 + x) mod 289
vec3 permute(vec3 x) {
    return mod((34.0 * x + 1.0) * x, 289.0);
}

// Cellular noise, returning F1 and F2 in a vec2.
// 3x3x3 search region for good F2 everywhere, but a lot
// slower than the 2x2x2 version.
// The code below is a bit scary even to its author,
// but it has at least half decent performance on a
// modern GPU. In any case, it beats any software
// implementation of Worley noise hands down.

vec2 cellular(vec3 P) {
#define K 0.142857142857 // 1/7
#define Ko 0.428571428571 // 1/2-K/2
#define K2 0.020408163265306 // 1/(7*7)
#define Kz 0.166666666667 // 1/6
#define Kzo 0.416666666667 // 1/2-1/6*2
#define jitter 1.0 // smaller jitter gives more regular pattern
    
	vec3 Pi = mod(floor(P), 289.0);
 	vec3 Pf = fract(P) - 0.5;
    
	vec3 Pfx = Pf.x + vec3(1.0, 0.0, -1.0);
	vec3 Pfy = Pf.y + vec3(1.0, 0.0, -1.0);
	vec3 Pfz = Pf.z + vec3(1.0, 0.0, -1.0);
    
	vec3 p = permute(Pi.x + vec3(-1.0, 0.0, 1.0));
	vec3 p1 = permute(p + Pi.y - 1.0);
	vec3 p2 = permute(p + Pi.y);
	vec3 p3 = permute(p + Pi.y + 1.0);
    
	vec3 p11 = permute(p1 + Pi.z - 1.0);
	vec3 p12 = permute(p1 + Pi.z);
	vec3 p13 = permute(p1 + Pi.z + 1.0);
    
	vec3 p21 = permute(p2 + Pi.z - 1.0);
	vec3 p22 = permute(p2 + Pi.z);
	vec3 p23 = permute(p2 + Pi.z + 1.0);
    
	vec3 p31 = permute(p3 + Pi.z - 1.0);
	vec3 p32 = permute(p3 + Pi.z);
	vec3 p33 = permute(p3 + Pi.z + 1.0);
    
	vec3 ox11 = fract(p11*K) - Ko;
	vec3 oy11 = mod(floor(p11*K), 7.0)*K - Ko;
	vec3 oz11 = floor(p11*K2)*Kz - Kzo; // p11 < 289 guaranteed
    
	vec3 ox12 = fract(p12*K) - Ko;
	vec3 oy12 = mod(floor(p12*K), 7.0)*K - Ko;
	vec3 oz12 = floor(p12*K2)*Kz - Kzo;
    
	vec3 ox13 = fract(p13*K) - Ko;
	vec3 oy13 = mod(floor(p13*K), 7.0)*K - Ko;
	vec3 oz13 = floor(p13*K2)*Kz - Kzo;
    
	vec3 ox21 = fract(p21*K) - Ko;
	vec3 oy21 = mod(floor(p21*K), 7.0)*K - Ko;
	vec3 oz21 = floor(p21*K2)*Kz - Kzo;
    
	vec3 ox22 = fract(p22*K) - Ko;
	vec3 oy22 = mod(floor(p22*K), 7.0)*K - Ko;
	vec3 oz22 = floor(p22*K2)*Kz - Kzo;
    
	vec3 ox23 = fract(p23*K) - Ko;
	vec3 oy23 = mod(floor(p23*K), 7.0)*K - Ko;
	vec3 oz23 = floor(p23*K2)*Kz - Kzo;
    
	vec3 ox31 = fract(p31*K) - Ko;
	vec3 oy31 = mod(floor(p31*K), 7.0)*K - Ko;
	vec3 oz31 = floor(p31*K2)*Kz - Kzo;
    
	vec3 ox32 = fract(p32*K) - Ko;
	vec3 oy32 = mod(floor(p32*K), 7.0)*K - Ko;
	vec3 oz32 = floor(p32*K2)*Kz - Kzo;
    
	vec3 ox33 = fract(p33*K) - Ko;
	vec3 oy33 = mod(floor(p33*K), 7.0)*K - Ko;
	vec3 oz33 = floor(p33*K2)*Kz - Kzo;
    
	vec3 dx11 = Pfx + jitter*ox11;
	vec3 dy11 = Pfy.x + jitter*oy11;
	vec3 dz11 = Pfz.x + jitter*oz11;
    
	vec3 dx12 = Pfx + jitter*ox12;
	vec3 dy12 = Pfy.x + jitter*oy12;
	vec3 dz12 = Pfz.y + jitter*oz12;
    
	vec3 dx13 = Pfx + jitter*ox13;
	vec3 dy13 = Pfy.x + jitter*oy13;
	vec3 dz13 = Pfz.z + jitter*oz13;
    
	vec3 dx21 = Pfx + jitter*ox21;
	vec3 dy21 = Pfy.y + jitter*oy21;
	vec3 dz21 = Pfz.x + jitter*oz21;
    
	vec3 dx22 = Pfx + jitter*ox22;
	vec3 dy22 = Pfy.y + jitter*oy22;
	vec3 dz22 = Pfz.y + jitter*oz22;
    
	vec3 dx23 = Pfx + jitter*ox23;
	vec3 dy23 = Pfy.y + jitter*oy23;
	vec3 dz23 = Pfz.z + jitter*oz23;
    
	vec3 dx31 = Pfx + jitter*ox31;
	vec3 dy31 = Pfy.z + jitter*oy31;
	vec3 dz31 = Pfz.x + jitter*oz31;
    
	vec3 dx32 = Pfx + jitter*ox32;
	vec3 dy32 = Pfy.z + jitter*oy32;
	vec3 dz32 = Pfz.y + jitter*oz32;
    
	vec3 dx33 = Pfx + jitter*ox33;
	vec3 dy33 = Pfy.z + jitter*oy33;
	vec3 dz33 = Pfz.z + jitter*oz33;
    
	vec3 d11 = dx11 * dx11 + dy11 * dy11 + dz11 * dz11;
	vec3 d12 = dx12 * dx12 + dy12 * dy12 + dz12 * dz12;
	vec3 d13 = dx13 * dx13 + dy13 * dy13 + dz13 * dz13;
	vec3 d21 = dx21 * dx21 + dy21 * dy21 + dz21 * dz21;
	vec3 d22 = dx22 * dx22 + dy22 * dy22 + dz22 * dz22;
	vec3 d23 = dx23 * dx23 + dy23 * dy23 + dz23 * dz23;
	vec3 d31 = dx31 * dx31 + dy31 * dy31 + dz31 * dz31;
	vec3 d32 = dx32 * dx32 + dy32 * dy32 + dz32 * dz32;
	vec3 d33 = dx33 * dx33 + dy33 * dy33 + dz33 * dz33;
    
	// Sort out the two smallest distances (F1, F2)
#if 0
	// Cheat and sort out only F1
	vec3 d1 = min(min(d11,d12), d13);
	vec3 d2 = min(min(d21,d22), d23);
	vec3 d3 = min(min(d31,d32), d33);
	vec3 d = min(min(d1,d2), d3);
	d.x = min(min(d.x,d.y),d.z);
	return sqrt(d.xx); // F1 duplicated, no F2 computed
#else
	// Do it right and sort out both F1 and F2
	vec3 d1a = min(d11, d12);
	d12 = max(d11, d12);
	d11 = min(d1a, d13); // Smallest now not in d12 or d13
	d13 = max(d1a, d13);
	d12 = min(d12, d13); // 2nd smallest now not in d13
	vec3 d2a = min(d21, d22);
	d22 = max(d21, d22);
	d21 = min(d2a, d23); // Smallest now not in d22 or d23
	d23 = max(d2a, d23);
	d22 = min(d22, d23); // 2nd smallest now not in d23
	vec3 d3a = min(d31, d32);
	d32 = max(d31, d32);
	d31 = min(d3a, d33); // Smallest now not in d32 or d33
	d33 = max(d3a, d33);
	d32 = min(d32, d33); // 2nd smallest now not in d33
	vec3 da = min(d11, d21);
	d21 = max(d11, d21);
	d11 = min(da, d31); // Smallest now in d11
	d31 = max(da, d31); // 2nd smallest now not in d31
	d11.xy = (d11.x < d11.y) ? d11.xy : d11.yx;
	d11.xz = (d11.x < d11.z) ? d11.xz : d11.zx; // d11.x now smallest
	d12 = min(d12, d21); // 2nd smallest now not in d21
	d12 = min(d12, d22); // nor in d22
	d12 = min(d12, d31); // nor in d31
	d12 = min(d12, d32); // nor in d32
	d11.yz = min(d11.yz,d12.xy); // nor in d12.yz
	d11.y = min(d11.y,d12.z); // Only two more to go
	d11.y = min(d11.y,d11.z); // Done! (Phew!)
	return sqrt(d11.xy); // F1, F2
#endif
}

// vec3 hotspot( vec2 loc, vec2 uv, vec3 color,
//               float intensity )
// {
//     float d = 1.0 / distance( loc, uv ) ;
//     float exponent = 2.0;
//     d = pow( d * intensity, exponent );
//     return vec3( color*d );
// }

float circle( vec2 position, vec2 center, float radius ) 
{
	return 1.0 - smoothstep( radius*0.95, radius, length(position-center) );
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
	vec3 liverSpecular = vec3( .35, .31, .28 );
    vec3 liverColor = vec3( 0.2, 0.137, 0.105 );
    vec3 dessicatedColor = vec3( 0.5, 0.4, 0.3 ) + vec3(0.05) * rand(f_vertexPosition.xy);
    vec3 vaporizingColor = vec3( 0.74, 0.651, 0.69 ) + vec3(0.2) * rand(f_vertexPosition.xy * u_time);
    vec3 vaporizedColor = liverColor + vec3(0.1);
    vec3 charredColor = vec3( 0.1, 0.025, 0.01 ) + vec3(0.2) * rand(f_vertexPosition.xy); 

    float blurWeight[25] = float[]( 1.0/273.0, 4.0/273.0, 7.0/273.0, 4.0/273.0, 1.0/273.0,
    	                            4.0/273.0,16.0/273.0,26.0/273.0,16.0/273.0, 4.0/273.0,
    	                            7.0/273.0,26.0/273.0,41.0/273.0,26.0/273.0, 7.0/273.0,
    	                            4.0/273.0,16.0/273.0,26.0/273.0,16.0/273.0, 4.0/273.0,
    	                            1.0/273.0, 4.0/273.0, 7.0/273.0, 4.0/273.0, 1.0/273.0 );

    vec3 tissueColor = vaporizingColor;
    // Don't blur vaporizing
    uint condCenter = texture( s_condition, f_blurTexCoords[12].xy ).r;
    if(  condCenter != 2u )
    {
        for( int i = 0; i < 25; ++i )
	    {
		    uint cond = texture( s_condition, f_blurTexCoords[i].xy ).r;

		    /// normalTissue=0, dessicatedTissue=1, vaporizingTissue=2, charredTissue=3.
			if( cond == 0u )
			{
				tissueColor += blurWeight[i] * liverColor;
			}
			if( cond == 1u )
			{
				tissueColor += blurWeight[i] * dessicatedColor;
			}
			if( cond == 2u ) 
			{
				tissueColor += blurWeight[i] * vaporizingColor;
			}
			if( cond == 3u ) 
			{
				tissueColor += blurWeight[i] * vaporizedColor;
			}
			if( cond == 4u )
			{
				tissueColor += blurWeight[i] * charredColor;
			}
		}
    }

    // define the scale of the pattern
    vec2 F = cellular( 80.0 * f_vertexPosition.xyz );
    // define the brightness
    float n = 0.4*(F.y-F.x) + 0.6;

	vec4 normal_camera = normalize( f_normal_camera + vec4(F.x, F.y, 0.5, 1) );
	vec4 toLight_camera = normalize( u_light.position_camera - f_vertex_camera );
	vec4 toCamera_camera = normalize( -f_vertex_camera );
	vec4 halfVector_camera = normalize( toCamera_camera + toLight_camera ); // Blinn's half-vector
	//vec4 reflect_camera = reflect( -toLight_camera, toCamera_camera ); // Phong's reflection model
    

	vec4 textureColor = vec4( n * tissueColor, 1.0 );
	vec4 Ia = u_ambientLight * textureColor;
	vec4 Id = u_light.diffuse * textureColor * max( dot (toLight_camera, normal_camera), 0.0 );
	vec4 Is = vec4( 1,1,1,1 ) * u_ks * pow( max(dot(halfVector_camera, normal_camera), 0.0), u_ns );
    
	outColor = Ia + Id + Is;

	//if( u_time > 0.5 && u_time < 7.5 )
	{
		vec4 targetColor = vec4( 1, 1, 0.3, 0.4 );
		vec2 target = vec2( 0.6, 0.35 );
		outColor += targetColor * circle( f_vertexPosition.xy, target, 0.04 ) - circle( f_vertexPosition.xy, target, 0.03 );
	}
}
