#version 120

uniform sampler2D uTexture;

varying vec3 vVertex;
varying vec3 vNormal;
varying vec2 vTexCoord;
varying vec4 vColor;

void main () {
    gl_FragColor = vec4(vNormal,1);
}
