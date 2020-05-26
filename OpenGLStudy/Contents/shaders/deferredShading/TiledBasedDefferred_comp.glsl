#version 430 compatibility
#extension GL_ARB_compute_shader: 					enable
#extension GL_ARB_shader_storage_buffer_object: 	enable
#extension GL_ARB_compute_variable_group_size : 	enable

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
	mat4 ProjectionMatrix;
	mat4 InvProj;
	mat4 ViewMatrix;
	mat4 InvView;
};
layout(std140, binding = 1) uniform uniformBuffer_drawcall
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};
struct Light{	
	vec3 color;
	bool enableShadow;
};

// Lighting, no interpoloation
struct AmbientLight{
	Light base;
};
struct DirectionalLight{
	Light base;
	vec3 direction;
};
struct PointLight{
	Light base;
	vec3 position;
	float radius;
	int ShadowMapIdx;	
	int ResolutionIdx;		
};
struct SpotLight{
	PointLight base;
	vec3 direction;
	float edge;
};
const int OMNI_SHADOW_MAP_COUNT = 5;
const int MAX_COUNT_PER_LIGHT = 5;
layout(std140, binding = 3) uniform g_uniformBuffer_Lighting
{
	SpotLight g_spotLights[MAX_COUNT_PER_LIGHT]; // 64 * MAX_COUNT_PER_LIGHT = 240 bytes
	PointLight g_pointLights[OMNI_SHADOW_MAP_COUNT * 16]; //  40 * 80 = 3.2 KB
	DirectionalLight g_directionalLight; // 32 bytes
	AmbientLight g_ambientLight; // 16 bytes	
	int g_pointLightCount; // 4 bytes
	int g_spotLightCount; // 4 bytes
}; 

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define WORK_GROUP_SIZE 16
#define NUM_GROUPS_X 80
#define NUM_GROUPS_Y 45

uniform sampler2D gDepth; // 3, depth buffer

// shared memory
shared uint minDepthInt = 0xFFFFFFFF;
shared uint maxDepthInt = 0;
shared uint visibleLightCount = 0;
shared uint visibleLightIndices[64];

// std430 will remove the restriction of rounding up to a multiple of 16 bytes like std140
layout(std430, binding = 3) buffer sLightData
{ 
	 uint g_visibleLightCount[NUM_GROUPS_X * NUM_GROUPS_Y];
};

layout(rgba32f, binding = 1) writeonly uniform image2D img_output;

layout( local_size_variable ) in;

//-----------------------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------------------

// this creates the standard Hessian-normal-form plane equation from three points, 
// except it is simplified for the case where the first point is the origin
vec4 CreatePlaneEquation( vec4 b, vec4 c )
{
    vec4 n;

    // normalize(cross( b.xyz-a.xyz, c.xyz-a.xyz )), except we know "a" is the origin
    n.xyz = normalize(cross( c.xyz, b.xyz ));

    // -(n dot a), except we know "a" is the origin
    n.w = 0;

    return n;
}

// point-plane distance, simplified for the case where 
// the plane passes through the origin
float GetSignedDistanceFromPlane( vec4 p, vec4 eqn )
{
	// https://mathworld.wolfram.com/Point-PlaneDistance.html
    // dot( eqn.xyz, p.xyz ) + eqn.w, , except we know eqn.w is zero 
    // (see CreatePlaneEquation above)
    return dot( eqn.xyz, p.xyz );
}

// calculate the number of tiles in the horizontal direction
uint GetNumTilesX()
{
    return uint( ( SCREEN_WIDTH + WORK_GROUP_SIZE - 1 ) / float(WORK_GROUP_SIZE) );
}

// calculate the number of tiles in the vertical direction
uint GetNumTilesY()
{
    return uint( ( SCREEN_HEIGHT + WORK_GROUP_SIZE - 1 ) / float(WORK_GROUP_SIZE) );
}

// convert a point from post-projection space into view space
vec4 ConvertProjToView( vec4 p )
{
    p = InvProj * p;
    p /= p.w;
    return p;
}

// convert a depth value from post-projection space into view space
float ConvertProjDepthToView( float z )
{
	float newZ = z;
    newZ = 1.f / (newZ*InvProj[3][2] + InvProj[3][3]);
    return newZ;
}

vec4 ViewPosFromDepth(vec2 texCoord, float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(texCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = InvProj * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;
    return viewSpacePosition;
}

void main()
{
	uint localIdxFlattened = gl_LocalInvocationID.x + gl_LocalInvocationID.y * WORK_GROUP_SIZE;
	if(localIdxFlattened == 0)
	{
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0;
		visibleLightCount = 0;
	}
	vec4 frustumEqn[4];
	{
		// construct frustum for this tile, getting rect coordinates of the tile, unit is pixel
    	uint minX = WORK_GROUP_SIZE * gl_WorkGroupID.x;
    	uint minY = WORK_GROUP_SIZE * gl_WorkGroupID.y;
    	uint maxX = WORK_GROUP_SIZE * (gl_WorkGroupID.x + 1);
   	 	uint maxY = WORK_GROUP_SIZE * (gl_WorkGroupID.y + 1);

		vec4 corners[4];
		// create a clock-wised square
		corners[0] = ConvertProjToView(vec4( (float(minX)/SCREEN_WIDTH) * 2.0f - 1.0f, 	(float(minY)/SCREEN_HEIGHT) * 2.0f - 1.0f, 1.0f, 1.0f));
 		corners[1] = ConvertProjToView(vec4( (float(maxX)/SCREEN_WIDTH) * 2.0f - 1.0f, 	(float(minY)/SCREEN_HEIGHT) * 2.0f - 1.0f, 1.0f, 1.0f));
 		corners[2] = ConvertProjToView(vec4( (float(maxX)/SCREEN_WIDTH) * 2.0f - 1.0f, 	(float(maxY)/SCREEN_HEIGHT) * 2.0f - 1.0f, 1.0f, 1.0f));
 		corners[3] = ConvertProjToView(vec4( (float(minX)/SCREEN_WIDTH) * 2.0f - 1.0f, 	(float(maxY)/SCREEN_HEIGHT) * 2.0f - 1.0f, 1.0f, 1.0f));

	    // create plane equations using the four corners of the tile, 
        for(uint i=0; i<4; i++)
            frustumEqn[i] = CreatePlaneEquation( corners[i], corners[(i+1)&3] );
	}
	
	// wait until all threads finish craeting frustums;
	barrier();

	vec2 texCoord = vec2(
		(gl_WorkGroupID.x * WORK_GROUP_SIZE + gl_LocalInvocationID.x) / float(SCREEN_WIDTH), 
		(gl_WorkGroupID.y * WORK_GROUP_SIZE + gl_LocalInvocationID.y) / float(SCREEN_HEIGHT)); 
	
	// get depth in float
	float depthFloat = texture(gDepth, texCoord).r;
	float viewPosZ = ConvertProjDepthToView( depthFloat );
	// convert depth from float to uint, since atomics only works on uints
	uint depthuInt = floatBitsToUint(abs(viewPosZ));
	
	// calculate min, max depth of this tile
	atomicMin(minDepthInt, depthuInt);
	atomicMax(maxDepthInt, depthuInt);

	// wait until min, max depth has been calculated in all threads of this tile
	barrier();

	float depthMinFloat = uintBitsToFloat(minDepthInt); // near to the camera
	float depthMaxFloat = uintBitsToFloat(maxDepthInt); // fat to the camera

	uint threadPertile = WORK_GROUP_SIZE * WORK_GROUP_SIZE;
	// loop over the lights and do a sphere vs. frustum intersection test
	// each thread process a point light in parallel, for max 80 point lights, i will end at 1
	for (uint i = 0; i < g_pointLightCount; i += threadPertile)
	{
		uint lightIndex = i + localIdxFlattened;
		if(lightIndex < g_pointLightCount)
		{
			vec4 pLightLocation = vec4(g_pointLights[lightIndex].position, 1.0);
			float r = g_pointLights[lightIndex].radius;
			// things that is in front of the camera in view space should have negative z value
			vec4 pLightLoc_ViewSpace = ViewMatrix * pLightLocation;
			// test if sphere is intersecting or inside frustum
			//if(pLightLoc_ViewSpace.z + r > depthMinFloat && pLightLoc_ViewSpace.z - r < depthMaxFloat)
			//if(pLightLoc_ViewSpace.z - r < -depthMinFloat && pLightLoc_ViewSpace.z + r > -depthMaxFloat)
			if(pLightLoc_ViewSpace.z - r < 0)
			{				
				float dist0 = GetSignedDistanceFromPlane( pLightLoc_ViewSpace, frustumEqn[0] );
				float dist1 = GetSignedDistanceFromPlane( pLightLoc_ViewSpace, frustumEqn[1] );
				float dist2 = GetSignedDistanceFromPlane( pLightLoc_ViewSpace, frustumEqn[2] );
				float dist3 = GetSignedDistanceFromPlane( pLightLoc_ViewSpace, frustumEqn[3] );
				if(
					( dist0 < r && dist0 > - r ) &&
                   	( dist1 < r && dist1 > - r ) &&
                    ( dist2 < r && dist2 > - r ) &&
                    ( dist3 < r && dist3 > - r ) )
				{
                    // do a thread-safe increment of the list counter 
                    // and put the index of this light into the list
                    uint id = atomicAdd(visibleLightCount, 1);
					visibleLightIndices[visibleLightCount] = lightIndex;
				}
                
			}
		}
	}

	barrier();
	uint workGroupID = gl_WorkGroupID.x + gl_WorkGroupID.y * NUM_GROUPS_X;
	g_visibleLightCount[workGroupID] = visibleLightCount;

	vec4 pixel = vec4(1.0, 1.0, 1.0, 1.0);
	if(visibleLightCount == 0)
		pixel = vec4(0,0,0,1);

	ivec2 pixel_coords = ivec2(gl_WorkGroupID.xy);
	imageStore(img_output, pixel_coords, pixel);
}
