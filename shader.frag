out vec4 fragColor;
// varying vec2 texCoord;

uniform vec2 resolution;

void main() {
    fragColor = vec4(gl_FragCoord.xy / resolution, 0.0, 1.0);
}
