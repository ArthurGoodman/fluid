varying out vec4 fragColor;

uniform sampler2D read;

uniform vec2 gridSize;
uniform float scale;

void main() {
    //I know, this is terrible...

    vec2 delta = vec2(0.0, 0.0);
    float actualScale = scale;

    bool onBoundary = false;

    if (int(gl_FragCoord.x) == 0) {
        delta += vec2(1.0, 0.0);
        onBoundary = true;
    } else if (int(gl_FragCoord.x) == int(gridSize.x - 1.0)) {
        delta += vec2(-1.0, 0.0);
        onBoundary = true;
    }

    if (int(gl_FragCoord.y) == 0) {
        delta += vec2(0.0, 1.0);
        onBoundary = true;
    } else if (int(gl_FragCoord.y) == int(gridSize.y - 1.0)) {
        delta += vec2(0.0, -1.0);
        onBoundary = true;
    }

    if (!onBoundary)
        actualScale = 1.0;

    fragColor = vec4(actualScale * texture2D(read, (gl_FragCoord.xy + delta) / gridSize).xy, 0.0, 1.0);
}
