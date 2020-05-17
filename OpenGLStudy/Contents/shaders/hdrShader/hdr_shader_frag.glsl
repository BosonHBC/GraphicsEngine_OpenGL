#version 420 core
const float gamma = 2.2;

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;

float g_lumaThreshold = 0.5f;
float g_mulReduceReciprocal = 8.0f;
float g_minReduceReciprocal = 128.0f;
float g_maxSpan = 8.0f;
const vec3 toLuma = vec3(0.299, 0.587, 0.114);

layout(std140, binding = 7) uniform g_uniformBuffer_postProcessing
{
    vec2 screenResolution;
	float exposure;
    int tonemappingMode;
    int EnablePostProcessing;
    int EnableFxAA;
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
// This method of compute fxaa is from McNopper
// https://github.com/McNopper/OpenGL/blob/master/Example42/shader/fxaa.frag.glsl
vec3 computeFxAA(vec3 color, vec2 texCoord)
{
    const vec2 texelStep = vec2( 1.0f / screenResolution.x , 1.0f / screenResolution.y);
    vec3 result = color;
    // Sampling neighbour texels. Offsets are adapted to OpenGL texture coordinates. 
	vec3 rgbNW = textureOffset(hdrBuffer, texCoord, ivec2(-1, 1)).rgb;
    vec3 rgbNE = textureOffset(hdrBuffer, texCoord, ivec2(1, 1)).rgb;
    vec3 rgbSW = textureOffset(hdrBuffer, texCoord, ivec2(-1, -1)).rgb;
    vec3 rgbSE = textureOffset(hdrBuffer, texCoord, ivec2(1, -1)).rgb;

    // Convert from RGB to luma.
	float lumaNW = dot(rgbNW, toLuma);
	float lumaNE = dot(rgbNE, toLuma);
	float lumaSW = dot(rgbSW, toLuma);
	float lumaSE = dot(rgbSE, toLuma);
	float lumaM = dot(color, toLuma);

    // Gather minimum and maximum luma.
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    // If contrast is lower than a maximum threshold ...
	if (lumaMax - lumaMin <= lumaMax * g_lumaThreshold)
	{
		return result;
	}

    // Sampling is done along the gradient.
	vec2 samplingDirection;	
	samplingDirection.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    samplingDirection.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    // Sampling step distance depends on the luma: The brighter the sampled texels, the smaller the final sampling step direction.
    // This results, that brighter areas are less blurred/more sharper than dark areas.  
    float samplingDirectionReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * g_mulReduceReciprocal, g_minReduceReciprocal);

    // Factor for norming the sampling direction plus adding the brightness influence. 
	float minSamplingDirectionFactor = 1.0 / (min(abs(samplingDirection.x), abs(samplingDirection.y)) + samplingDirectionReduce);

    // Calculate final sampling direction vector by reducing, clamping to a range and finally adapting to the texture size. 
    samplingDirection = clamp(samplingDirection * minSamplingDirectionFactor, vec2(-g_maxSpan), vec2(g_maxSpan)) * texelStep;

    // Inner samples on the tab.
	vec3 rgbSampleNeg = texture(hdrBuffer, texCoord + samplingDirection * (1.0/3.0 - 0.5)).rgb;
	vec3 rgbSamplePos = texture(hdrBuffer, texCoord + samplingDirection * (2.0/3.0 - 0.5)).rgb;

	vec3 rgbTwoTab = (rgbSamplePos + rgbSampleNeg) * 0.5;

    // Outer samples on the tab.
	vec3 rgbSampleNegOuter = texture(hdrBuffer, texCoord + samplingDirection * (0.0/3.0 - 0.5)).rgb;
	vec3 rgbSamplePosOuter = texture(hdrBuffer, texCoord + samplingDirection * (3.0/3.0 - 0.5)).rgb;
	
	vec3 rgbFourTab = (rgbSamplePosOuter + rgbSampleNegOuter) * 0.25 + rgbTwoTab * 0.5;
	
    // Calculate luma for checking against the minimum and maximum value.
	float lumaFourTab = dot(rgbFourTab, toLuma);

    // Are outer samples of the tab beyond the edge ... 
	if (lumaFourTab < lumaMin || lumaFourTab > lumaMax)
	{
		// ... yes, so use only two samples.
		result = rgbTwoTab; 
        
	}
	else
	{
		// ... no, so use four samples. 
		result = rgbFourTab;
        
	}

    return result;
}
void main()
{             
    vec2 texCoord = TexCoords;
    texCoord.y = 1-texCoord.y;
    vec3 hdrColor = texture(hdrBuffer, texCoord).rgb;
     vec3 result = hdrColor;
    if(EnablePostProcessing == 1)
    {
        if(EnableFxAA == 1)
        {
            result = computeFxAA(result, texCoord);
        }
        // exposure
        result = vec3(1.0) - exp(-result * exposure);
        
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
    }
    FragColor = vec4(result, 1.0);
}

