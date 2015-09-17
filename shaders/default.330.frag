#version 330

uniform vec3 eyeVector = vec3(0,0,1);
uniform sampler2D uTexture;

in vec3 vVertex;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vColor;

out vec4 fColor;

void main () {
    float ambientFactor = 0.1;
    float diffuseFactor = max(0, dot(vNormal, eyeVector));
    float specularFactor = 0.0;

    vec4 texColor = texture2D(uTexture, vTexCoord);
    //~ fColor = vColor * max(0.1, dot(vNormal, vec3(0,0,1)));
    //~ fColor = vColor * texColor;
    //~ fColor = vec4(0.5 * (vNormal + 1), 1);
    texColor = max(vec4(1.0), texColor);

    fColor = ambientFactor + vColor * texColor * (diffuseFactor + specularFactor);

    fColor.rgb = fColor.ggg;
}
