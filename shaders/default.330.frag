#version 330

uniform sampler2D uTexture;

in vec3 vVertex;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vColor;

out vec4 oColor;

void main () {
    vec4 texColor = texture2D(uTexture, vTexCoord);
    //~ gl_FragCoord = vScreenVertex;
    /*oColor*/ gl_FragColor = /*vColor * texColor*/ vec4(0.5 * (vNormal + 1), 1);
}
