varying out vec4 fragColor;

uniform sampler2D read;

uniform vec2 gridSize;

uniform vec3 color;
uniform vec2 point;
uniform float radius;

float gauss(vec2 p, float r) {
    return exp(-dot(p, p) / r);
}

void main() {
    vec2 uv = gl_FragCoord.xy / gridSize;
    vec3 base = texture2D(read, uv).xyz;
    vec2 coord = point.xy - gl_FragCoord.xy;
    vec3 splat = color * gauss(coord, gridSize.x * radius);
    fragColor = vec4(base + splat, 1.0);
}
