#version 330

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
const mat4 cProjMatrix = mat4(2/16,0,0,0, 0,2/9,0,0, 0,0,-2/20,0, 0,0,0,1);

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;
in vec4 aColor;

out vec3 vVertex;
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 vColor;

void main() {
    vec4 vertex = uViewMatrix * vec4(aVertex,1);
    vec4 normal = uViewMatrix * vec4(aNormal,0);
    gl_Position = uProjMatrix * vertex;
    vVertex = vertex.xyz;
    vNormal = normalize(normal.xyz);
    vTexCoord = aTexCoord;
    vColor = aColor;
}
