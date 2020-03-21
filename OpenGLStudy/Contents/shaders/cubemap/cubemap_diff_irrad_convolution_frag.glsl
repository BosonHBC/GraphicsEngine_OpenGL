#version 420
const float PI = 3.14159265359;
out vec4 outColor;
in vec3 TexCoords;

uniform samplerCube cubemapTex; // 0

void main()
{    
        
     vec3 N = normalize(TexCoords);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, N);
    up            = cross(N, right);
       
    float sampleDelta = 0.025;
    int nrSamples = 0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 
            
            vec3 irradColor = textureLod(cubemapTex, normalize(sampleVec), 0.0).rgb;
            if(isnan(irradColor).r || isnan(irradColor).g || isnan(irradColor).b) 
                irradColor = vec3(0.5,0.5,0.5);
            irradiance += cos(theta) * sin(theta) * irradColor;
            nrSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    outColor = vec4(irradiance, 1.0);
}