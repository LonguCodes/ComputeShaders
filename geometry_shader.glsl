#version 430 core
layout (points) in;
layout (line_strip, max_vertices = 17) out;

#define pi 3.141592653589793238462643383279

uniform mat4 P;
uniform float pointSize;

void main() {    

    for(int i =0;i<17;i++){
        gl_Position = P*( gl_in[0].gl_Position + vec4(sin(i/8.0*pi), cos(i/8.0*pi), 0.0, 0.0)*pointSize);
        EmitVertex();
    }
    EndPrimitive();
}  