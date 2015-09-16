#version 330

//~ #error

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;

attribute vec3 aVertex;
attribute vec3 aNormal;
attribute vec2 aTexCoord;
attribute vec4 aColor;

out vec3 vVertex;
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 vColor;

void main() {
    vec4 vertex = uViewMatrix * vec4(aVertex, 1);
    vec4 normal = uViewMatrix * vec4(aNormal, 0);
    gl_Position = uProjMatrix * vertex;
    vVertex = vertex.xyz;
    vNormal = normalize(normal.xyz);
    vTexCoord = aTexCoord;
    vColor = aColor;
}
