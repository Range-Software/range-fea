#version 120

uniform mat4 uProjection;
uniform mat4 uModelView;

attribute vec3  aPosition;
attribute vec4  aColor;      // UNSIGNED_BYTE, normalised to [0,1] by GL driver
attribute float aTexCoord;

varying vec4  vColor;
varying float vTexCoord;

void main()
{
    vec4 eyePos = uModelView * vec4(aPosition, 1.0);
    gl_Position = uProjection * eyePos;
    gl_ClipVertex = eyePos;
    vColor      = aColor;
    vTexCoord   = aTexCoord;
}
