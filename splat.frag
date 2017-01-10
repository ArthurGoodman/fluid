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

uniform sampler2D read;

uniform vec2 gridSize;

uniform vec3 color;
uniform vec2 point;
uniform float radius;

float gauss(vec2 p, float r) {
    return exp(-dot(p, p) / r);
}

void main() {
    vec2 uv = gl_FragCoord.xy / gridSize.xy;
    vec3 base = vec3(unpack(texture2D(read, uv)), 0.0);
    vec2 coord = point.xy - gl_FragCoord.xy;
    vec3 splat = color * gauss(coord, gridSize.x * radius);
    fragColor = pack((base + splat).xy);
}
