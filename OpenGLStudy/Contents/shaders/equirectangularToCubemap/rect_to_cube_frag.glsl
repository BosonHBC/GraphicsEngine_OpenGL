#version 420
out vec4 outColor;

in vec3 TexCoords;

uniform sampler2D rectangularHDRMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{    
    vec3 texCoord = TexCoords;
    texCoord.y *= -1;
    vec2 uv = SampleSphericalMap(normalize(texCoord));
    vec3 color = texture(rectangularHDRMap, uv).rgb;
    
    outColor = vec4(color, 1.0);
}