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
    vec4 params2;   // x = clipping enabled, y = point size
    vec4 params3;   // x = point quad expansion, y = viewport width, z = viewport height
} ubuf;

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

    if (!useLighting || numLights == 0)
    {
        fragColor = baseColor;
        return;
    }

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

    fragColor = vec4(result.rgb, baseColor.a);
}
