#version 120

varying vec4  vColor;
varying vec3  vNormal;
varying vec3  vFragPos;
varying float vTexCoord;

uniform bool      uUseTexture;
uniform bool      uUseLighting;
uniform bool      uTwoSided;
uniform bool      uHighlight;
uniform sampler1D uColorMap;

struct Light { vec3 position; vec4 ambient; vec4 diffuse; };

// Fraction by which a highlighted (selected) entity is moved towards white.
const float highlightAmount = 0.2;
// Glow added to a highlighted entity, strongest where its surface turns away from the viewer.
const vec3  glowColor    = vec3(0.4, 0.75, 1.0);
const float glowStrength = 0.8;
// Glow of elements without a surface normal (lines, points).
const float glowFlat     = 0.25;

uniform int   uNumLights;
uniform Light uLights[8];

void main()
{
    // Back faces: silver for solid surfaces; two-sided (cut planes, iso surfaces) use element colour.
    // vTexCoord < 0 is a sentinel: use vertex colour (e.g. black edges over a textured face).
    vec4 baseColor = (!gl_FrontFacing && !uTwoSided)
                     ? vec4(0.75, 0.75, 0.75, vColor.a)
                     : ((uUseTexture && vTexCoord >= 0.0) ? texture1D(uColorMap, vTexCoord) : vColor);
    vec3 color = baseColor.rgb;
    if (uUseLighting && uNumLights > 0)
    {
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
        color = result.rgb;
    }
    if (uHighlight)
    {
        color = mix(color, vec3(1.0), highlightAmount);
        // Projection is orthographic, the viewer looks along the eye space z axis.
        float rim = glowFlat;
        if (length(vNormal) > 1.0e-6)
        {
            rim = pow(1.0 - abs(normalize(vNormal).z), 2.0);
        }
        color = min(color + glowColor * glowStrength * rim, vec3(1.0));
    }
    gl_FragColor = vec4(color, baseColor.a);
}
