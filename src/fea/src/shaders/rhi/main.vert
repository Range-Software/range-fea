#version 440

// Vertex attribute layout — must match RhiRenderer::acquirePipeline().
layout(location = 0) in vec3  aPosition;
layout(location = 1) in vec3  aNormal;
layout(location = 2) in vec4  aColor;      // UNORM8 x 4, normalised to [0,1]
layout(location = 3) in float aTexCoord;

layout(location = 0) out vec4  vColor;
layout(location = 1) out vec3  vNormal;
layout(location = 2) out float vTexCoord;
layout(location = 3) out float vClipDist;

// Uniform block — must match struct RhiUniformBlock and main.frag.
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

void main()
{
    vec4 eyePosition = ubuf.modelView * vec4(aPosition, 1.0);

    gl_Position  = ubuf.mvp * vec4(aPosition, 1.0);
    gl_PointSize = ubuf.params2.y;

    // An expanded point carries the corner of its quad in the normal, as a
    // sign per axis. The quad is sized here rather than where it was built
    // so that it stays the requested number of pixels across whatever the
    // projection does with the point. NDC spans the viewport over 2.0, so
    // half of a size of p pixels is p / viewport away from the centre.
    if (ubuf.params3.x > 0.5)
    {
        vec2 halfSize = vec2(ubuf.params2.y) / vec2(ubuf.params3.y, ubuf.params3.z);
        gl_Position.xy += aNormal.xy * halfSize * gl_Position.w;
    }

    vNormal   = mat3(ubuf.normalMatrix) * aNormal;
    vColor    = aColor;
    vTexCoord = aTexCoord;

    // Clipping is done in the fragment shader — QRhi has no fixed clip planes.
    vClipDist = (ubuf.params2.x > 0.5) ? dot(ubuf.clipPlane, eyePosition) : 1.0;
}
