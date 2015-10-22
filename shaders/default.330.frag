#version 330
#extension all : warn

precision mediump float;

uniform float uTime = 0;
uniform vec2 uResolution = vec2(1);

uniform vec3 uEyePos = vec3(0,0,5);

uniform vec3 uLightDir  = vec3(0,1,0);
uniform vec3 uLightPos  = vec3(-4,4,4);
uniform vec3 uLightAmbt = vec3(0.5, 0.5, 0.5);
uniform vec3 uLightDiff = vec3(0.5, 0.5, 0.5);
uniform vec3 uLightSpec = vec3(1.0, 1.0, 1.0);

uniform sampler2D uDiffMap; // diffuse color (albedo)
uniform sampler2D uSpecMap; // specular color + glossiness
uniform sampler2D uGlowMap; // emission color + AO
uniform sampler2D uBumpMap; // normal + height
uniform float uSpecPow = 100.0; // specular exponent
uniform vec2 uBumpScaleBias = vec2(0.04, 0.02);

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

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord )
{
    // assume N, the interpolated vertex normal and
    // V, the view vector (vertex to eye)
    vec3 map = texture2D( uBumpMap, texcoord ).xyz;/*
#ifdef WITH_NORMALMAP_UNSIGNED
    map = map * 255./127. - 128./127.;
#endif
#ifdef WITH_NORMALMAP_2CHANNEL
    map.z = sqrt( 1. - dot( map.xy, map.xy ) );
#endif
#ifdef WITH_NORMALMAP_GREEN_UP
    map.y = -map.y;
#endif*/
    mat3 TBN = cotangent_frame( N, -V, texcoord );
    return normalize( TBN * map );
}

////////////////////////////////////////////////////////////////////////////////

void main () {
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vVertex);
    vec3 eyeDir = normalize(uEyePos - vVertex);

    vec2 texCoord = vTexCoord;
    float height = texture2D(uBumpMap, texCoord).w;

    mat3 TBN = cotangent_frame( normal, -eyeDir, texCoord );
    float scale = uBumpScaleBias.x * height - uBumpScaleBias.y;
    //~ texCoord += scale * normalize(TBN * eyeDir).xy;

    //~ bumpTexVal = texture2D(uBumpMap, texCoord);
    //~ normal = normalize(normal + (bumpTexVal.xzy * 2 - 1));
    //~ normal = perturb_normal( normal, eyeDir, vTexCoord );

    normal = normalize(TBN * (texture2D(uBumpMap, texCoord).xyz * 2 - 1));

    vec3 halfVec = normalize(eyeDir + lightDir);

    float diffFactor = max(0, dot(normal, lightDir));
    float specFactor = max(0, dot(normal, halfVec)); // Blinn-Phong
    //~ float specFactor = max(0, dot(eyeDir, reflect(lightDir, normal))); // True Phong
    //~ float diffFactor = max(0, dot(normal, uLightDir));
    //~ float specFactor = max(0, dot(normal, normalize(eyeDir+uLightDir)));

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
    fColor.rgb += specColor * pow(specFactor, specPower);
    fColor.rgb += glowColor;

    //~ fColor.rgb = 0.5 + 0.5 * normal;

    //~ fColor.rgb = vec3(diffFactor, specFactor, 0.0);
    //~ fColor.rgb = vec3(diffFactor);
    //~ fColor.r = diffFactor;

    //~ fColor.rg = gl_FragCoord.xy / uResolution;
}
