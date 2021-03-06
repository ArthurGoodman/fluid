varying out vec4 fragColor;

uniform sampler2D read;
uniform sampler2D density;
uniform sampler2D temperature;

uniform vec2 gridSize;

uniform float timestep;
uniform float k;
uniform float buoyancyFactor;

void main() {
    vec2 uv = gl_FragCoord.xy / gridSize;
    fragColor = vec4(texture2D(read, uv).xy + timestep * (-k * texture2D(density, uv).x + buoyancyFactor * texture2D(temperature, uv).x) * vec2(0.0, 1.0), 0.0, 1.0);
}
