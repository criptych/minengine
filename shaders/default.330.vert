#version 330
#extension all : warn

uniform float uTime = 0.0;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;

uniform float uGlowPulseBias = 0.6;
uniform float uGlowPulseScale = 0.4;
uniform float uGlowPulseRate = 0.5;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;

out vec3 vVertex;
out vec3 vNormal;
out vec2 vTexCoord;
out float vGlowFactor;

void main() {
    vec4 vertex = uViewMatrix * vec4(aVertex, 1);
    vec4 normal = uViewMatrix * vec4(aNormal, 0);
    gl_Position = uProjMatrix * vertex;
    vVertex = vertex.xyz;
    vNormal = normalize(normal.xyz);
    vTexCoord = aTexCoord;

    vGlowFactor = uGlowPulseBias + uGlowPulseScale * 0.5 *
        (1.0 - cos(uTime * uGlowPulseRate * 6.2831853071));
}
