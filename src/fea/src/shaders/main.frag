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
// Number of equal bands of a stepped color map, 0 for a continuous one.
uniform int       uBandCount;
// Draw lines between the bands of a stepped color map.
uniform bool      uBandOutline;

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

// Color of a stepped color map: band color with smooth edges and lines of constant width on screen.
vec4 findBandColor(float texCoord)
{
    float nBands = float(uBandCount);
    float scaled = clamp(texCoord, 0.0, 1.0) * nBands;
    // Change of band position over one pixel.
    float pixel  = max(fwidth(scaled), 1.0e-6);
    float edge   = floor(scaled + 0.5);

    // No line at the minimum and the maximum of the color map.
    if (edge < 0.5 || edge > nBands - 0.5)
    {
        return texture1D(uColorMap, (min(floor(scaled), nBands - 1.0) + 0.5) / nBands);
    }

    vec4  lower    = texture1D(uColorMap, (edge - 0.5) / nBands);
    vec4  upper    = texture1D(uColorMap, (edge + 0.5) / nBands);
    float edgeOffset = scaled - edge;
    vec4  color    = mix(lower, upper, smoothstep(-0.5 * pixel, 0.5 * pixel, edgeOffset));

    // Constant values have no edge to draw, lines fade out when bands get only a few pixels wide.
    if (uBandOutline && pixel > 1.0e-5)
    {
        vec3  average   = 0.5 * (lower.rgb + upper.rgb);
        float luminance = dot(average, vec3(0.299, 0.587, 0.114));
        vec3  lineColor = (luminance < 0.15) ? mix(average, vec3(1.0), 0.35) : 0.5 * average;
        float lineAlpha = (1.0 - smoothstep(0.25, 1.0, abs(edgeOffset) / pixel)) * (1.0 - smoothstep(0.15, 0.4, pixel));
        color.rgb = mix(color.rgb, lineColor, lineAlpha);
    }
    return color;
}

void main()
{
    // Back faces: silver for solid surfaces; two-sided (cut planes, iso surfaces) use element colour.
    // vTexCoord < 0 is a sentinel: use vertex colour (e.g. black edges over a textured face).
    vec4 baseColor = vColor;
    if (!gl_FrontFacing && !uTwoSided)
    {
        baseColor = vec4(0.75, 0.75, 0.75, vColor.a);
    }
    else if (uUseTexture && vTexCoord >= 0.0)
    {
        baseColor = (uBandCount > 0) ? findBandColor(vTexCoord) : texture1D(uColorMap, vTexCoord);
    }
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
