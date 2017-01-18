varying out vec4 fragColor;

uniform sampler2D read;

uniform vec2 gridSize;
uniform vec2 offset;
uniform float scale;

void main() {
    fragColor = vec4(scale * texture2D(read, (gl_FragCoord.xy + offset) / gridSize).xyz, 1.0);
}
