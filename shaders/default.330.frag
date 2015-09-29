#version 330
#extension all : warn

precision mediump float;

uniform float uTime = 0;
uniform vec2 uResolution = vec2(1);

uniform vec3 uEyePos = vec3(0,0,5);

uniform vec3 uLightPos  = vec3(-4,4,4);
uniform vec4 uLightAmbt = vec4(0.4, 0.4, 1.0, 1.0);
uniform vec4 uLightDiff = vec4(1.0, 0.4, 0.4, 1.0);
uniform vec4 uLightSpec = vec4(0.4, 1.0, 0.4, 1.0);

uniform sampler2D uDiffMap;
uniform sampler2D uSpecMap;
uniform sampler2D uGlowMap;

in vec3 vVertex;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vColor;

out vec4 fColor;

void main () {
    vec3 normal = normalize(vNormal);
    vec3 lightDir = uLightPos - vVertex;
    vec3 eyeDir = uEyePos - vVertex;

    float diffFactor = max(0, dot(normal, normalize(lightDir)));
    float specFactor = max(0, dot(normal, normalize(eyeDir+lightDir)));

    vec4 diffTexCol = vec4(1.0,1.0,1.0,1.0); //texture2D(uDiffMap, vTexCoord);
    vec4 specTexCol = vec4(1.0,1.0,1.0,0.3); //texture2D(uSpecMap, vTexCoord);
    vec4 glowTexCol = vec4(0.0,0.0,0.0,1.0); //texture2D(uGlowMap, vTexCoord);

    vec4 ambtColor = uLightAmbt * diffTexCol * vColor;
    vec4 diffColor = uLightDiff * diffTexCol * vColor;
    vec4 specColor = uLightSpec * specTexCol;
    float specPower = pow(10.0, 10.0*specTexCol.a);
    vec4 glowColor = glowTexCol * glowTexCol.a;

    fColor  = ambtColor;
    fColor += diffColor * diffFactor;
    fColor += specColor * pow(specFactor, specPower);
    fColor += glowColor;
    fColor.a = vColor.a * diffTexCol.a;

    fColor.rgb = 0.5 + 0.5 * normal;

    //~ fColor.rg = gl_FragCoord.xy / uResolution;
}
