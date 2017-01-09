uniform sampler2D read;
uniform vec3 bias;
uniform vec3 scale;
uniform vec2 resolution;

void main() {
    gl_FragColor = vec4(bias + scale * texture2D(read, gl_FragCoord.xy / resolution.xy).xyz, 1.0);
}
