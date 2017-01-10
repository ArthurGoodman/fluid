out vec4 fragColor;

#extension GL_ARB_shading_language_packing : enable

vec2 unpack(vec4 rgba) {
    uvec4 t = uvec4(rgba * 255.0);

    uint v = t.x;
    v *= 256u;
    v += t.y;
    v *= 256u;
    v += t.z;
    v *= 256u;
    v += t.w;

    return unpackHalf2x16(v);
}

vec4 pack(vec2 v) {
    uint t = packHalf2x16(v);

    uint a = t % 256u;
    t /= 256u;
    uint b = t % 256u;
    t /= 256u;
    uint g = t % 256u;
    t /= 256u;
    uint r = t % 256u;

    return vec4(r, g, b, a) / 255.0;
}

uniform sampler2D velocity;
uniform sampler2D advected;

uniform vec2 gridSize;
uniform float gridScale;

uniform float timestep;
uniform float dissipation;

vec2 bilerp(sampler2D d, vec2 p) {
    vec4 ij; // i0, j0, i1, j1
    ij.xy = floor(p - 0.5) + 0.5;
    ij.zw = ij.xy + 1.0;

    vec4 uv = ij / gridSize.xyxy;
    vec2 d11 = unpack(texture2D(d, uv.xy));
    vec2 d21 = unpack(texture2D(d, uv.zy));
    vec2 d12 = unpack(texture2D(d, uv.xw));
    vec2 d22 = unpack(texture2D(d, uv.zw));

    vec2 a = p - ij.xy;

    return mix(mix(d11, d21, a.x), mix(d12, d22, a.x), a.y);
}

void main() {
    vec2 uv = gl_FragCoord.xy / gridSize.xy;
    float scale = 1.0 / gridScale;

    // trace point back in time
    vec2 p = gl_FragCoord.xy - timestep * scale * unpack(texture2D(velocity, uv));

    fragColor = pack(dissipation * bilerp(advected, p));
}
