#version 330
out uvec4 fragColor;

#extension GL_EXT_gpu_shader4 : enable

uniform usampler2D read;
uniform vec3 bias;
uniform vec3 scale;
uniform vec2 resolution;

void main() {
    fragColor = floatBitsToUint(vec4(bias + scale * uintBitsToFloat(texture2D(read, gl_FragCoord.xy / resolution.xy).xyz), 1.0)) * 255u;
}
