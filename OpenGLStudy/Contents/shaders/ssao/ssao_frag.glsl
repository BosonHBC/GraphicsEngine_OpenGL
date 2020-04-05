#version 420 core
in vec2 TexCoords;
out float FragColor;
layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
	mat4 ProjectionMatrix;
	mat4 InvProj;
	mat4 ViewMatrix;
	mat4 InvView;
};
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); 
const int SSAO_MAX_SAMPLECOUNT = 64;
const float bias = 0.025;
layout(std140, binding = 8) uniform uniformBuffer_ssao
{
	vec4 Samples[SSAO_MAX_SAMPLECOUNT];
	int SampeCount;
	float radius;
	float power;
};
uniform sampler2D gNormalRoughness; // 0
uniform sampler2D gDepth;           // 1
uniform sampler2D texNoise;         // 2

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
    vec2 texCoord = TexCoords;
    //texCoord.y = 1.0 - texCoord.y;
    float depth = texture(gDepth, texCoord).r;
	vec3 viewPos = ViewPosFromDepth(texCoord, depth).xyz;
	vec3 viewNormal = normalize(( inverse(transpose(ViewMatrix)) * vec4(texture(gNormalRoughness, texCoord).rgb, 1.0)).xyz);

    vec3 randomVec = normalize(texture(texNoise, texCoord * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - viewNormal * dot(randomVec, viewNormal));
    vec3 bitangent = cross(viewNormal, tangent);
    mat3 TBN = mat3(tangent, bitangent, viewNormal);

    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < SampeCount; ++i)
    {
        // get sample position
        vec3 _sample = TBN * Samples[i].xyz; // from tangent to view-space
        _sample = viewPos + _sample * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(_sample, 1.0);
        offset = ProjectionMatrix * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float offsetDepth = texture(gDepth, offset.xy).r;
        vec3 offsetViewPos = ViewPosFromDepth(texCoord, offsetDepth).xyz;
        float sampleDepth = offsetViewPos.z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - sampleDepth));
        occlusion += (sampleDepth >= _sample.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / SampeCount);

    FragColor = pow(occlusion, power);
}