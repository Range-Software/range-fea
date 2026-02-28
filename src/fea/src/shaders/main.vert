#version 120

uniform mat4 uProjection;
uniform mat4 uModelView;

attribute vec3  aPosition;
attribute vec3  aNormal;
attribute vec4  aColor;     // UNSIGNED_BYTE, normalised to [0,1] by GL driver
attribute float aTexCoord;

varying vec4  vColor;
varying vec3  vNormal;
varying vec3  vFragPos;
varying float vTexCoord;

void main()
{
    vec4 worldPos = uModelView * vec4(aPosition, 1.0);
    gl_Position   = uProjection * worldPos;

    vFragPos  = worldPos.xyz;
    vNormal   = mat3(uModelView) * aNormal;
    vColor    = aColor;
    vTexCoord = aTexCoord;
}
