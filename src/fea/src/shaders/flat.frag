#version 120

varying vec4  vColor;
varying float vTexCoord;

uniform bool      uUseTexture;
uniform sampler1D uColorMap;

void main()
{
    gl_FragColor = uUseTexture ? texture1D(uColorMap, vTexCoord) : vColor;
}
