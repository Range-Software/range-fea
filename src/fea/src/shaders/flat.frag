#version 130

in vec4  vColor;
in float vTexCoord;

uniform bool      uUseTexture;
uniform sampler1D uColorMap;

void main()
{
    gl_FragColor = uUseTexture ? texture(uColorMap, vTexCoord) : vColor;
}
