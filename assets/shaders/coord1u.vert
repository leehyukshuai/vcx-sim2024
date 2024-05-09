#version 410 core

layout(location = 0) in  vec3 a_Position;
layout(location = 1) in  vec3 a_Color;

layout(location = 0) out vec3 v_Color;

uniform mat4  u_Transform;

void main() {
    gl_Position = u_Transform * vec4(a_Position, 1.);
    v_Color     = a_Color;
}
