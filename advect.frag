#version 330
out uvec4 fragColor;

#extension GL_EXT_gpu_shader4 : enable

uniform usampler2D velocity;
uniform usampler2D advected;

uniform vec2 gridSize;
uniform float gridScale;

uniform float timestep;
uniform float dissipation;

vec2 bilerp(usampler2D d, vec2 p) {
    vec4 ij; // i0, j0, i1, j1
    ij.xy = floor(p - 0.5) + 0.5;
    ij.zw = ij.xy + 1.0;

    vec4 uv = ij / gridSize.xyxy;
    vec2 d11 = uintBitsToFloat(texture2D(d, uv.xy).xy);
    vec2 d21 = uintBitsToFloat(texture2D(d, uv.zy).xy);
    vec2 d12 = uintBitsToFloat(texture2D(d, uv.xw).xy);
    vec2 d22 = uintBitsToFloat(texture2D(d, uv.zw).xy);

    vec2 a = p - ij.xy;

    return mix(mix(d11, d21, a.x), mix(d12, d22, a.x), a.y);
}

void main() {
    vec2 uv = gl_FragCoord.xy / gridSize.xy;
    float scale = 1.0 / gridScale;

    // trace point back in time
    vec2 p = gl_FragCoord.xy - timestep * scale * uintBitsToFloat(texture2D(velocity, uv).xy);

    fragColor = floatBitsToUint(vec4(dissipation * bilerp(advected, p), 0.0, 1.0)) * 255u;
}
