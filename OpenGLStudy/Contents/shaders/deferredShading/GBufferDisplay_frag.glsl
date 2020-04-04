#version 420 core
const float maxIORinNature = 3.927; // Gallium(III) Arsenide 
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gAlbedoMetallic;
uniform sampler2D gNormalRoughness;
uniform sampler2D gIOR;
uniform sampler2D gDepth;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
	mat4 ProjectionMatrix;
	mat4 InvProj;
	mat4 ViewMatrix;
	mat4 InvView;
};

vec4 ViewPosFromDepth(vec2 texCoord, float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(texCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = InvProj * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;
    return viewSpacePosition;
}
uniform int displayMode;
void main()
{             
    vec2 texCoord = TexCoords;
    texCoord.y = 1-texCoord.y;
    switch(displayMode)
    {
        case 0:
            FragColor = vec4(texture(gAlbedoMetallic, texCoord).rgb, 1.0);
        break;
        case 1:
            FragColor = vec4(vec3(texture(gAlbedoMetallic, texCoord).a), 1.0);
        break;
        case 2:
            FragColor = vec4(vec3(texture(gNormalRoughness, texCoord).a), 1.0);
        break;
        case 3:
            vec3 _normal = texture(gNormalRoughness, texCoord).rgb;
            _normal = 0.5 * (_normal + vec3(1,1,1));
            FragColor = vec4(_normal, 1.0);
        break;
        case 4:
            FragColor = vec4(texture(gIOR, texCoord).rgb/maxIORinNature, 1.0);
        break;
        case 5:
            FragColor = vec4(vec3(texture(gDepth, texCoord).r-0.9) * 10, 1.0);
        break;
        case 6:
            float depth = texture(gDepth, texCoord).r;
	        vec4 ViewSpacePos = ViewPosFromDepth(texCoord, depth);
            //worldPos /= 500.f;
           // worldPos = (worldPos)/ 2.f;
            FragColor = vec4((InvView * ViewSpacePos).xyz , 1.0);
        break;
        default:
        FragColor = vec4(0,0,0,1);
        break;                   
    }
}