#version 130

in vec4  vColor;
in vec3  vNormal;
in vec3  vFragPos;
in float vTexCoord;

uniform bool      uUseTexture;
uniform bool      uUseLighting;
uniform sampler1D uColorMap;

// Up to 8 lights — matches the fixed-function GL_LIGHT0..GL_LIGHT7 limit.
struct Light
{
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
};

uniform int   uNumLights;
uniform Light uLights[8];

void main()
{
    vec4 baseColor = uUseTexture ? texture(uColorMap, vTexCoord) : vColor;

    if (!uUseLighting || uNumLights == 0)
    {
        gl_FragColor = baseColor;
        return;
    }

    vec3 norm   = normalize(vNormal);
    // Two-sided lighting: flip normal for back faces.
    float facing = gl_FrontFacing ? 1.0 : -1.0;
    vec4 result = vec4(0.0);

    for (int i = 0; i < uNumLights; i++)
    {
        // Directional light (w == 0): position is already the direction vector.
        vec3  lightDir = normalize(uLights[i].position);
        float diff     = max(dot(norm * facing, lightDir), 0.0);
        result += uLights[i].ambient  * baseColor;
        result += diff * uLights[i].diffuse * baseColor;
    }

    gl_FragColor = vec4(result.rgb, baseColor.a);
}
