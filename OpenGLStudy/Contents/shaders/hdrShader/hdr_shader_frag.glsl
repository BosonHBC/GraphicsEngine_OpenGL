#version 420 core
const float gamma = 2.2;

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;

layout(std140, binding = 7) uniform g_uniformBuffer_postProcessing
{
	float exposure;
    bool EnablePostProcessing;
};

void main()
{             
    vec2 texCoord = TexCoords;
    texCoord.y = 1-texCoord.y;
    vec3 hdrColor = texture(hdrBuffer, texCoord).rgb;
    if(EnablePostProcessing)
    {
        // reinhard
        // vec3 result = hdrColor / (hdrColor + vec3(1.0));
        // exposure
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
        // also gamma correct while we're at it       
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
    else
    {
        vec3 result = pow(hdrColor, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
}