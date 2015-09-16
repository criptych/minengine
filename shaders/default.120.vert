#version 120

varying vec3 vVertex;
varying vec3 vNormal;
varying vec2 vTexCoord;
varying vec4 vColor;

void main() {
    vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
    vec4 normal = gl_ModelViewMatrix * vec4(gl_Normal,0);
    gl_Position = gl_ProjectionMatrix * vertex;
    vVertex = vertex.xyz;
    vNormal = normalize(normal.xyz);
    vTexCoord = gl_MultiTexCoord0.st;
    vColor = gl_Color;
}
