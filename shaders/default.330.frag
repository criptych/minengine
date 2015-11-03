#version 330
#extension all : warn

precision mediump float;

uniform float uTime = 0.0;
uniform vec2 uResolution = vec2(1);

struct Light {
    vec3 position;
    vec4 ambtColor;
    vec4 diffColor;
    vec4 specColor;
};

uniform Light uLights[4];

struct Material {
    sampler2D diffMap;
    sampler2D specMap;
    sampler2D glowMap;
    sampler2D bumpMap;
    float specPower;
    float bumpScale;
    float bumpBias;
    float fresnelPower;
    float fresnelScale;
    float fresnelBias;
};

uniform Material uMaterial;

uniform vec4 uFogColor = vec4(vec3(0.0), 1.0);
uniform float uFogDensity = 0.2;
uniform vec2 uFogRange = vec2(5.0, 50.0);

in vec3 vVertex;
in vec3 vNormal;
in vec2 vTexCoord;

in float vGlowFactor;

out vec4 fColor;

////////////////////////////////////////////////////////////////////////////////
// source: http://www.thetenthplanet.de/archives/1180
////////////////////////////////////////////////////////////////////////////////

mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );

    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

////////////////////////////////////////////////////////////////////////////////

void main () {
    vec3 normal = normalize(vNormal);
    vec3 eyeDir = normalize(-vVertex);

    vec2 texCoord = vTexCoord;
    float height = texture2D(uMaterial.bumpMap, texCoord).w;

    mat3 TBN = cotangent_frame( normal, -eyeDir, texCoord );
    float scale = uMaterial.bumpScale * height - uMaterial.bumpBias;
    texCoord += scale * normalize(TBN * eyeDir).xy;

    vec4 diffTexCol = texture2D(uMaterial.diffMap, texCoord);
    vec4 specTexCol = texture2D(uMaterial.specMap, texCoord);
    vec4 glowTexCol = texture2D(uMaterial.glowMap, texCoord);

    vec3 bumpNormal = texture2D(uMaterial.bumpMap, texCoord).xyz * 2 - 1;
    normal = normalize(TBN * bumpNormal);

    vec3 ambtColor, diffColor, specColor;
    vec3 glowColor = glowTexCol.rgb;

    glowColor *= vGlowFactor;

    for (int i = 0; i < 4; i++) {

        vec3 lightDir = normalize(uLights[i].position.xyz - vVertex);
        vec3 halfVec = normalize(eyeDir + lightDir);

        float diffFactor = max(0, dot(normal, lightDir));
        float specFactor = max(0, dot(normal, halfVec)); // Blinn-Phong
        //~ float specFactor = max(0, dot(eyeDir, reflect(lightDir, normal))); // True Phong
        specFactor = pow(specFactor, uMaterial.specPower);

        float fresnelFactor = uMaterial.fresnelBias + uMaterial.fresnelScale *
            pow(1.0 - dot(eyeDir, halfVec), uMaterial.fresnelPower);

        vec3 ambtLight = uLights[i].ambtColor.rgb;
        vec3 diffLight = uLights[i].diffColor.rgb * (1.0f - fresnelFactor);
        vec3 specLight = uLights[i].specColor.rgb * (0.0f + fresnelFactor);

        ambtColor += ambtLight * diffTexCol.rgb * glowTexCol.a;
        diffColor += diffLight * diffTexCol.rgb * diffFactor;
        specColor += specLight * specTexCol.rgb * specFactor * specTexCol.a;
    }

    fColor = vec4(0);
    fColor.rgb += ambtColor;
    fColor.rgb += diffColor;
    fColor.rgb += specColor;
    fColor.rgb += glowColor;
    fColor.a   += diffTexCol.a;

    float fogFactor = exp(-pow(uFogDensity*max(0.0, -vVertex.z-uFogRange.x), 2.0));
    //~ float fogFactor = exp(-uFogDensity*max(0.0, -vVertex.z-uFogRange.x));
    //~ float fogFactor = 1.0 - ((-vVertex.z - uFogRange.x) / (uFogRange.y - uFogRange.x));
    fColor.rgb = mix(uFogColor.rgb, fColor.rgb, clamp(fogFactor, 0.0, 1.0));

    fColor.rgb = pow(fColor.rgb, vec3(1.0/2.2));
}
