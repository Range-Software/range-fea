#version 130

// For unlit geometry (wireframes, axes, text anchors, etc.)
uniform mat4 uProjection;
uniform mat4 uModelView;

out vec4  vColor;
out float vTexCoord;

void main()
{
    gl_Position = uProjection * uModelView * gl_Vertex;
    vColor      = gl_Color;
    vTexCoord   = gl_MultiTexCoord0.s;
}
