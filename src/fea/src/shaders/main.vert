#version 130

// Projection and modelview matrices — replaces the deprecated GL matrix stack.
// Uploaded each frame by GLWidget::drawModel() via GLShaderProgram.
uniform mat4 uProjection;
uniform mat4 uModelView;

// Per-fragment outputs
out vec4  vColor;
out vec3  vNormal;
out vec3  vFragPos;
out float vTexCoord;

void main()
{
    // gl_Vertex / gl_Normal / gl_Color / gl_MultiTexCoord0 are the legacy
    // built-in attribute aliases fed by glVertexPointer / glNormalPointer /
    // glColorPointer / glTexCoordPointer (OpenGL compatibility profile).
    vec4 worldPos = uModelView * gl_Vertex;
    gl_Position   = uProjection * worldPos;

    vFragPos  = worldPos.xyz;
    vNormal   = mat3(transpose(inverse(mat3(uModelView)))) * gl_Normal;
    vColor    = gl_Color;
    vTexCoord = gl_MultiTexCoord0.s;
}
