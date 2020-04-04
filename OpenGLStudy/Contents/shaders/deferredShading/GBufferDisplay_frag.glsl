#version 420 core
const float maxIORinNature = 3.927; // Gallium(III) Arsenide 
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gAlbedoMetallic;
uniform sampler2D gNormalRoughness;
uniform sampler2D gIOR;
uniform sampler2D gDepth;

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
        default:
        FragColor = vec4(0,0,0,1);
        break;                   
    }
}