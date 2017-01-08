varying vec2 texCoord;

void main() {
    texCoord = gl_Vertex.xy;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
