#version 440

layout(location = 0) in vec4  vColor;
layout(location = 1) in vec3  vNormal;
layout(location = 2) in float vTexCoord;
layout(location = 3) in float vClipDist;

layout(location = 0) out vec4 fragColor;

// Uniform block — must match struct RhiUniformBlock and main.vert.
layout(std140, binding = 0) uniform Ubuf {
    mat4 mvp;
    mat4 modelView;
    mat4 normalMatrix;
    vec4 clipPlane;
    vec4 lightPosition[8];
    vec4 lightAmbient[8];
    vec4 lightDiffuse[8];
    vec4 params;    // x = number of lights, y = use texture, z = use lighting, w = two sided
    vec4 params2;   // x = clipping enabled, y = point size, z = highlight
    vec4 params3;   // x = point quad expansion, y = viewport width, z = viewport height
} ubuf;

// Fraction by which a highlighted (selected) entity is moved towards white.
const float highlightAmount = 0.2;
// Glow added to a highlighted entity, strongest where its surface turns away from the viewer.
const vec3  glowColor    = vec3(0.4, 0.75, 1.0);
const float glowStrength = 0.8;
// Glow of elements without a surface normal (lines, points).
const float glowFlat     = 0.25;

// Colour map — an Nx1 strip standing in for the 1D texture of the OpenGL backend.
layout(binding = 1) uniform sampler2D uColorMap;

void main()
{
    if (vClipDist < 0.0)
    {
        discard;
    }

    int  numLights   = int(ubuf.params.x + 0.5);
    bool useTexture  = ubuf.params.y  > 0.5;
    bool useLighting = ubuf.params.z  > 0.5;
    bool twoSided    = ubuf.params.w  > 0.5;

    // Back faces: silver for solid surfaces; two-sided (cut planes, iso surfaces) use element colour.
    // vTexCoord < 0 is a sentinel: use vertex colour (e.g. black edges over a textured face).
    vec4 baseColor = (!gl_FrontFacing && !twoSided)
                     ? vec4(0.75, 0.75, 0.75, vColor.a)
                     : ((useTexture && vTexCoord >= 0.0)
                        ? texture(uColorMap, vec2(clamp(vTexCoord, 0.0, 1.0), 0.5))
                        : vColor);

    vec3 color = baseColor.rgb;

    if (useLighting && numLights > 0)
    {
        // Flip the normal for back faces so lighting looks correct from the viewer's side.
        vec3 norm = gl_FrontFacing ? normalize(vNormal) : normalize(-vNormal);

        vec4 result = vec4(0.0);
        for (int i = 0; i < numLights; i++)
        {
            vec3  lightDir = normalize(ubuf.lightPosition[i].xyz);
            float diff     = max(dot(norm, lightDir), 0.0);
            result += ubuf.lightAmbient[i] * baseColor;
            result += diff * ubuf.lightDiffuse[i] * baseColor;
        }
        color = result.rgb;
    }

    if (ubuf.params2.z > 0.5)
    {
        color = mix(color, vec3(1.0), highlightAmount);
        // Projection is orthographic, the viewer looks along the eye space z axis.
        // Expanded point quads carry the quad corner in the normal, not a surface normal.
        float rim = glowFlat;
        if (ubuf.params3.x < 0.5 && length(vNormal) > 1.0e-6)
        {
            rim = pow(1.0 - abs(normalize(vNormal).z), 2.0);
        }
        color = min(color + glowColor * glowStrength * rim, vec3(1.0));
    }

    fragColor = vec4(color, baseColor.a);
}
