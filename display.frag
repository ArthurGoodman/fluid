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
uniform vec3 bias;
uniform vec3 scale;
uniform vec2 resolution;

void main() {
    fragColor = vec4(bias + scale * vec3(unpack(texture2D(read, gl_FragCoord.xy / resolution.xy)), 0.0), 1.0);
}
