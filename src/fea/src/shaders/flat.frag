#version 120

varying vec4  vColor;
varying float vTexCoord;

uniform bool      uUseTexture;
uniform sampler1D uColorMap;

void main()
{
    // vTexCoord < 0 is a sentinel: use vertex colour, not the texture.
    gl_FragColor = (uUseTexture && vTexCoord >= 0.0) ? texture1D(uColorMap, vTexCoord) : vColor;
}
