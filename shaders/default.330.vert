#version 330

//~ #error

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;
in vec4 aColor;

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
