#version 130

// For unlit geometry (wireframes, axes, text anchors, etc.)
uniform mat4 uProjection;
uniform mat4 uModelView;

// Vertex attributes — fed by glVertexAttribPointer (replaces deprecated client arrays).
in vec3  aPosition;   // layout location 0
in vec4  aColor;      // layout location 2 — UNSIGNED_BYTE normalised to [0,1]
in float aTexCoord;   // layout location 3

out vec4  vColor;
out float vTexCoord;

void main()
{
    gl_Position = uProjection * uModelView * vec4(aPosition, 1.0);
    vColor      = aColor;
    vTexCoord   = aTexCoord;
}
