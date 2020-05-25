#version 430 compatibility
#extension GL_ARB_compute_shader: 					enable
#extension GL_ARB_shader_storage_buffer_object: 	enable
#extension GL_ARB_compute_variable_group_size : 	enable

const vec3 g = vec3( 0., -90.8, 0. );
const float dt = 0.01;
const float groundY = 1;
const float firctionCoefficient = 0.5;
#define DAMPING -0.1f

layout( std140, binding=0 ) buffer sPos
{
	vec4 Positions[ ];
};

layout( std140, binding=1 ) buffer sVel
{
	vec4 Velocities[ ];
};

layout( std140, binding=2 ) buffer sOtherData
{
	vec4 OtherData[];
};

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

float randomRange(float min, float max, float rnd)
{
	return rnd * (max - min) + min;
}
uniform float g_time;
uniform float g_lifeTime;
uniform float g_delayTime;
uniform vec3 g_initialLocMin;
uniform vec3 g_initialLocMax;
uniform vec3 g_initialVelMin;
uniform vec3 g_initialVelMax;

layout( local_size_variable ) in;

void main()
{
	uint gid = gl_GlobalInvocationID.x; // the .y and .z are both 1 in this case
	vec3 p = Positions[ gid ].xyz;
	vec3 v = Velocities[ gid ].xyz;
	float elpasedLifeTime = OtherData[gid].x;
	
	if(elpasedLifeTime >= 0)
	{
		if(elpasedLifeTime < g_lifeTime)
		{
			vec3 a = DAMPING * v;
			if(p.y > groundY)
			{
				a += g;
			}
			else
			{
				// ground friction
				a += firctionCoefficient * (-g.y) * -normalize(vec3(v.x, 0, v.z));
			}
			vec3 pp = p + v*dt + 0.5*dt*dt*a;
			vec3 vp = v + a * dt;

			if(pp.y <= groundY)
			{
				pp.y = groundY;
				vp.y = abs(vp.y) * 0.5f;
				if(vp.y <= 1.f)
					vp.y = 0;
			}

			Positions[ gid ].xyz = pp;
			Velocities[ gid ].xyz = vp;
		}
		else
		{
			// if life time is over
			float rndx = random(vec2(Positions[gid].x, g_time));
			float rndy = random(vec2(Positions[gid].y, g_time));
			float rndz = random(vec2(Positions[gid].z, g_time));

			Positions[gid].xyz = vec3(
				randomRange(g_initialLocMin.x, g_initialLocMax.x, rndx),	
				randomRange(g_initialLocMin.y, g_initialLocMax.y, rndy),	
				randomRange(g_initialLocMin.z, g_initialLocMax.z, rndz));

			Velocities[gid].xyz = vec3(
				randomRange(g_initialVelMin.x, g_initialVelMax.x, rndx),
				randomRange(g_initialVelMin.y, g_initialVelMax.y, rndy),	
				randomRange(g_initialVelMin.z, g_initialVelMax.z, rndz));
			elpasedLifeTime = randomRange(g_delayTime,0, random(vec4(rndx, rndy, rndz, g_time)));
		}
	}
	elpasedLifeTime += dt;

	OtherData[gid].x = elpasedLifeTime;
}