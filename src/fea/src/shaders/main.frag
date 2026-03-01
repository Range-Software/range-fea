#version 120

varying vec4  vColor;
varying vec3  vNormal;
varying vec3  vFragPos;
varying float vTexCoord;

uniform bool      uUseTexture;
uniform bool      uUseLighting;
uniform bool      uTwoSided;
uniform sampler1D uColorMap;

struct Light { vec3 position; vec4 ambient; vec4 diffuse; };

uniform int   uNumLights;
uniform Light uLights[8];

void main()
{
    // Back faces: silver for solid surfaces; two-sided (cut planes, iso surfaces) use flipped normal.
    if (!gl_FrontFacing && !uTwoSided) { gl_FragColor = vec4(0.75, 0.75, 0.75, 1.0); return; }
    // vTexCoord < 0 is a sentinel: use vertex colour (e.g. black edges over a textured face).
    vec4 baseColor = (uUseTexture && vTexCoord >= 0.0) ? texture1D(uColorMap, vTexCoord) : vColor;
    if (!uUseLighting || uNumLights == 0) { gl_FragColor = baseColor; return; }
    // Flip normal for back faces so lighting looks correct from the viewer's side.
    vec3 norm = gl_FrontFacing ? normalize(vNormal) : normalize(-vNormal);
    vec4 result = vec4(0.0);
    for (int i = 0; i < uNumLights; i++)
    {
        vec3  lightDir = normalize(uLights[i].position);
        float diff     = max(dot(norm, lightDir), 0.0);
        result += uLights[i].ambient  * baseColor;
        result += diff * uLights[i].diffuse * baseColor;
    }
    gl_FragColor = vec4(result.rgb, baseColor.a);
}
