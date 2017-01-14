varying out vec4 fragColor;

uniform sampler2D velocity;

uniform vec2 gridSize;
uniform float gridScale;

void main() {
    vec2 uv = gl_FragCoord.xy / gridSize;

    vec2 xOffset = vec2(1.0 / gridSize.x, 0.0);
    vec2 yOffset = vec2(0.0, 1.0 / gridSize.y);

    float vl = texture2D(velocity, uv - xOffset).x;
    float vr = texture2D(velocity, uv + xOffset).x;
    float vb = texture2D(velocity, uv - yOffset).y;
    float vt = texture2D(velocity, uv + yOffset).y;

    float gridScale = 0.5 / gridScale;
    float divergence = gridScale * (vr - vl + vt - vb);

    fragColor = vec4(divergence, 0.0, 0.0, 1.0);
}
