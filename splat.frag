#version 330
out uvec4 fragColor;

#extension GL_EXT_gpu_shader4 : enable

uniform usampler2D read;

uniform vec2 gridSize;

uniform vec3 color;
uniform vec2 point;
uniform float radius;

float gauss(vec2 p, float r) {
    return exp(-dot(p, p) / r);
}

void main() {
    vec2 uv = gl_FragCoord.xy / gridSize.xy;
    vec3 base = uintBitsToFloat(texture2D(read, uv).xyz);
    vec2 coord = point.xy - gl_FragCoord.xy;
    vec3 splat = color * gauss(coord, gridSize.x * radius);
    fragColor = floatBitsToUint(vec4(base + splat, 1.0)) * 255u;
}
