varying out vec4 fragColor;

uniform sampler2D read;
uniform vec2 resolution;

uniform mat4 scale;

void main() {
    fragColor = vec4(texture2D(read, gl_FragCoord.xy / resolution.xy).xyz, 1.0) * scale;
}
