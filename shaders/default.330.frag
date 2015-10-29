#version 330
#extension all : warn

precision mediump float;

uniform float uTime = 0;
uniform vec2 uResolution = vec2(1);
uniform vec3 uEyePos = vec3(0,0,5);

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

in vec3 vVertex;
in vec3 vNormal;
in vec2 vTexCoord;

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
    vec3 eyeDir = normalize(uEyePos - vVertex);

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

    for (int i = 0; i < 4; i++) {

        vec3 lightDir = normalize(uLights[i].position.xyz - vVertex /* uLight.position.w/**/);
        vec3 halfVec = normalize(eyeDir + lightDir);

        float diffFactor = max(0, dot(normal, lightDir));
        //~ float diffFactor = max(0, dot(normal, uLightDir));
        float specFactor = pow(max(0, dot(normal, halfVec)), /*uMaterial.specPower*/10.0); // Blinn-Phong
        //~ float specFactor = pow(max(0, dot(eyeDir, reflect(lightDir, normal))), uMaterial.specPower); // True Phong
        //~ float specFactor = max(0, dot(normal, normalize(eyeDir+uLightDir)));

        float fresnelFactor = uMaterial.fresnelBias + uMaterial.fresnelScale *
            pow(1.0 - dot(eyeDir, halfVec), uMaterial.fresnelPower);

        //~ diffFactor = pow(0.5 + 0.5 * diffFactor, 2.0);

        ambtColor += uLights[i].ambtColor.rgb * diffTexCol.rgb * glowTexCol.a;
        diffColor += uLights[i].diffColor.rgb * diffTexCol.rgb * diffFactor
                                                        * (1.0f - fresnelFactor);
        specColor += uLights[i].specColor.rgb * specTexCol.rgb * specTexCol.a * specFactor
                                                        * (0.0f + fresnelFactor);
    }

    fColor = vec4(0);
    fColor.rgb += diffColor;
    fColor.rgb += specColor;
    fColor.rgb += glowColor;
    fColor.a   += diffTexCol.a;



    //~ fColor.rgb = 0.5 + 0.5 * normal;

    //~ fColor.rgb = vec3(diffFactor, specFactor, 0.0);
    //~ fColor.rgb = vec3(diffFactor);
    //~ fColor.rgb = vec3(specFactor);
    //~ fColor.r = diffFactor;
    //~ fColor.rgb = vec3(diffFactor);
    //~ fColor.rgb = vec3(dFdx( vTexCoord ) * 10 + 0.5, 0);

    //~ fColor.rgb = normal * 0.5 + 0.5;
    //~ fColor.rgb = vec3(diffFactor);

    //~ fColor.rgb = vec3(texCoord, 0);
    //~ fColor.rgb = vec3(vTexCoord, 0);
    //~ fColor.rgb = bumpNormal * 0.5 + 0.5;

    //~ fColor.rgb = vec3(glowTexCol.a);

    //~ fColor.rg = gl_FragCoord.xy / uResolution;

    //~ fColor.rgb = fresnelFactor * vec3(1,0,0) + dot(halfVec, normal) * vec3(0,1,0);// + specFactor * vec3(0,0,1);
    //~ fColor.rgb = mix(vec3(diffFactor, 0, 0), vec3(0, specFactor, 0), fresnelFactor);

    //~ fColor.rgb = vec3(fresnelFactor);

    //~ fColor.rgb = fresnelFactor * (0.5 + 0.5 * normal);

    //~ fColor = vec4(vec3(gl_FragDepth), 1);
}
