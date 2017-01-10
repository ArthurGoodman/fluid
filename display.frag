#version 330
out vec4 fragColor;

uniform sampler2D read;
uniform vec3 bias;
uniform vec3 scale;
uniform vec2 resolution;

void main() {
    fragColor = vec4(bias + scale * texture2D(read, gl_FragCoord.xy / resolution.xy).xyz, 1.0);
}
