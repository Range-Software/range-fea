#version 130

// Projection and modelview matrices — replaces the deprecated GL matrix stack.
// Uploaded each frame by GLWidget::drawModel() via GLShaderProgram.
uniform mat4 uProjection;
uniform mat4 uModelView;

// Clip plane — replaces deprecated glClipPlane() / GL_CLIP_PLANE0.
uniform vec4 uClipPlane;    // Eye-space clip plane equation (xyz=normal, w=offset).
uniform bool uClipEnabled;  // Whether clipping is active.

// Vertex attributes — fed by glVertexAttribPointer (replaces deprecated client arrays).
in vec3  aPosition;   // layout location 0
in vec3  aNormal;     // layout location 1
in vec4  aColor;      // layout location 2 — UNSIGNED_BYTE normalised to [0,1]
in float aTexCoord;   // layout location 3

// Per-fragment outputs
out vec4  vColor;
out vec3  vNormal;
out vec3  vFragPos;
out float vTexCoord;

void main()
{
    vec4 worldPos = uModelView * vec4(aPosition, 1.0);
    gl_Position   = uProjection * worldPos;

    // Write clip distance — positive = inside, negative = clipped.
    gl_ClipDistance[0] = uClipEnabled ? dot(worldPos, uClipPlane) : 1.0;

    vFragPos  = worldPos.xyz;
    vNormal   = mat3(transpose(inverse(mat3(uModelView)))) * aNormal;
    vColor    = aColor;
    vTexCoord = aTexCoord;
}
