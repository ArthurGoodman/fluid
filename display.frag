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

uniform sampler2D read;
uniform vec3 bias;
uniform vec3 scale;
uniform vec2 resolution;

void main() {
    // fragColor = vec4(bias + scale * texture2D(read, gl_FragCoord.xy / resolution.xy).xyz, 1.0);
    fragColor = vec4(unpack(read, gl_FragCoord.xy / resolution.xy) * 0.5 + 0.5, 0.5, 1.0);
}
