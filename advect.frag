out vec4 fragColor;

vec2 encode(float v) {
    vec2 enc = vec2(1.0, 255.0) * v;
    enc = fract(enc);
    enc -= enc.yy * vec2(1.0 / 255.0, 0.0);
    return enc;
}

float decode(vec2 v) {
    return dot(v, vec2(1.0, 1.0 / 255.0));
}

vec2 unpack(sampler2D s, vec2 uv) {
    vec4 c = texture2D(s, uv);
    return vec2((decode(c.xy) - 0.5) / 0.5, (decode(c.zw) - 0.5) / 0.5);
}

vec4 pack(vec2 v) {
    v *= 0.5;
    v += 0.5;
    return vec4(encode(v.x), encode(v.y));
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
    vec2 d11 = unpack(d, uv.xy);
    vec2 d21 = unpack(d, uv.zy);
    vec2 d12 = unpack(d, uv.xw);
    vec2 d22 = unpack(d, uv.zw);

    vec2 a = p - ij.xy;

    return mix(mix(d11, d21, a.x), mix(d12, d22, a.x), a.y);
}

void main() {
    vec2 uv = gl_FragCoord.xy / gridSize.xy;
    float scale = 1.0 / gridScale;

    // trace point back in time
    vec2 p = gl_FragCoord.xy - timestep * scale * unpack(velocity, uv);

    fragColor = pack(dissipation * bilerp(advected, p));
}
