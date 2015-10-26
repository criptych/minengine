#version 330
#extension all : warn

precision mediump float;

uniform float uTime = 0;
uniform vec2 uResolution = vec2(1);

uniform vec3 uEyePos = vec3(0,0,5);

uniform vec3 uLightDir  = vec3(0,1,0);
uniform vec3 uLightPos  = vec3(-4,4,4);
//~ uniform vec3 uLightAmbt = vec3(0.2, 0.2, 0.2);
uniform vec3 uLightAmbt = vec3(0.5, 0.5, 0.5);
//~ uniform vec3 uLightAmbt = vec3(0.8, 0.8, 0.8);
//~ uniform vec3 uLightAmbt = vec3(1.0, 1.0, 1.0);
//~ uniform vec3 uLightDiff = vec3(0.2, 0.2, 0.2);
uniform vec3 uLightDiff = vec3(0.5, 0.5, 0.5);
//~ uniform vec3 uLightDiff = vec3(0.8, 0.8, 0.8);
//~ uniform vec3 uLightDiff = vec3(1.0, 1.0, 1.0);
uniform vec3 uLightSpec = vec3(1.0, 1.0, 1.0);

uniform sampler2D uDiffMap; // diffuse color (albedo)
uniform sampler2D uSpecMap; // specular color + glossiness
uniform sampler2D uGlowMap; // emission color + AO
uniform sampler2D uBumpMap; // normal + height
uniform float uSpecPow = 100.0; // specular exponent
uniform float uFresnelBias = 0.05; // fresnel bias
uniform float uFresnelScale = 0.95; // fresnel scaling
uniform float uFresnelPow = 5.0; // fresnel exponent
uniform float uBumpScale = 0.04; // displacement scaling
uniform float uBumpBias = 0.00; // displacement bias

in vec3 vVertex;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vColor;

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
    vec3 lightDir = normalize(uLightPos - vVertex);
    vec3 eyeDir = normalize(uEyePos - vVertex);

    vec2 texCoord = vTexCoord;
    float height = texture2D(uBumpMap, texCoord).w;

    mat3 TBN = cotangent_frame( normal, -eyeDir, texCoord );
    float scale = uBumpScale * height - uBumpBias;
    texCoord += scale * normalize(TBN * eyeDir).xy;

    vec3 bumpNormal = texture2D(uBumpMap, texCoord).xyz * 2 - 1;
    normal = normalize(TBN * bumpNormal);
    //~ normal = bumpNormal;

    vec3 halfVec = normalize(eyeDir + lightDir);

    float diffFactor = max(0, dot(normal, lightDir));
    //~ float diffFactor = max(0, dot(normal, uLightDir));
    float specFactor = max(0, dot(normal, halfVec)); // Blinn-Phong
    //~ float specFactor = max(0, dot(eyeDir, reflect(lightDir, normal))); // True Phong
    //~ float specFactor = max(0, dot(normal, normalize(eyeDir+uLightDir)));

    float fresnelFactor = uFresnelBias + uFresnelScale * pow(1.0 + dot(eyeDir, normal), uFresnelPow);
    fresnelFactor = min(1.0, max(0.0, fresnelFactor));

    vec4 diffTexCol = texture2D(uDiffMap, texCoord); //vec4(1.0,1.0,1.0,1.0);
    vec4 specTexCol = texture2D(uSpecMap, texCoord); //vec4(1.0,1.0,1.0,0.2);
    vec4 glowTexCol = texture2D(uGlowMap, texCoord); //vec4(0.0,0.0,0.0,1.0);

    //~ diffFactor = pow(0.5 + 0.5 * diffFactor, 2.0);

    diffTexCol *= vColor;

    vec3 ambtColor = uLightAmbt * diffTexCol.rgb * glowTexCol.a;
    vec3 diffColor = uLightDiff * diffTexCol.rgb;
    vec3 specColor = uLightSpec * specTexCol.rgb;
    float specPower = uSpecPow * specTexCol.a;
    vec3 glowColor = glowTexCol.rgb;

    fColor = vec4(ambtColor, diffTexCol.a);
    fColor.rgb += diffColor * diffFactor;
    fColor.rgb += specColor * pow(specFactor, specPower) * specTexCol.a;
    fColor.rgb += specColor * fresnelFactor * specTexCol.a;
    fColor.rgb += glowColor;

    //~ fColor.rgb = 0.5 + 0.5 * normal;

    //~ fColor.rgb = vec3(diffFactor, specFactor, 0.0);
    //~ fColor.rgb = vec3(diffFactor);
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

    //~ fColor.rgb = vec3(fresnelFactor);

    //~ fColor = vec4(vec3(gl_FragDepth), 1);
}
