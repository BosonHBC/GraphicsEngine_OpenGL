#version 420 core
const float gamma = 2.2;

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;

layout(std140, binding = 7) uniform g_uniformBuffer_postProcessing
{
	float exposure;
    int tonemappingMode;
    bool EnablePostProcessing;
};

// this three tone mapping method is from JoshuaSenouf
// https://github.com/JoshuaSenouf/gl-engine
vec3 ReinhardTM(vec3 color)
{
    return color / (color + vec3(1.0f));
}

vec3 FilmicTM(vec3 color)
{
    color = max(vec3(0.0f), color - vec3(0.004f));
    color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f) + 0.06f);

    return color;
}

vec3 UnchartedTM(vec3 color)
{
  const float A = 0.15f;
  const float B = 0.50f;
  const float C = 0.10f;
  const float D = 0.20f;
  const float E = 0.02f;
  const float F = 0.30f;
  const float W = 11.2f;

  color = ((color * (A * color + C * B) + D * E) / (color * ( A * color + B) + D * F)) - E / F;

  return color;
}

void main()
{             
    vec2 texCoord = TexCoords;
    texCoord.y = 1-texCoord.y;
    vec3 hdrColor = texture(hdrBuffer, texCoord).rgb;
    if(EnablePostProcessing)
    {
        // exposure
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
        
         // Tonemapping computation
        if(tonemappingMode == 0)
        {
            result = ReinhardTM(result);
            result = pow(result, vec3(1.0 / gamma));
        }
        else if(tonemappingMode == 1)
        {
            result = FilmicTM(result);
        }
        else if(tonemappingMode == 2)
        {
            const float W = 11.2f;
            result = UnchartedTM(result);
            vec3 whiteScale = 1.0f / UnchartedTM(vec3(W));

            result *= whiteScale;
            result = pow(result, vec3(1.0 / gamma));
        }

        // also gamma correct while we're at it       

        FragColor = vec4(result, 1.0);
    }
    else
    {
        FragColor = vec4(hdrColor, 1.0);
    }
}

