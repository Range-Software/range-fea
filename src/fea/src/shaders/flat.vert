#version 120

// For unlit geometry (wireframes, axes, text anchors, etc.)
uniform mat4 uProjection;
uniform mat4 uModelView;

attribute vec3  aPosition;
attribute vec4  aColor;
attribute float aTexCoord;

varying vec4  vColor;
varying float vTexCoord;

void main()
{
    gl_Position = uProjection * uModelView * vec4(aPosition, 1.0);
    vColor      = aColor;
    vTexCoord   = aTexCoord;
}
