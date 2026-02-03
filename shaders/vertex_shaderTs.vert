#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 pValid;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 cam;

out vec2 TexCoord;

void main(){
    uint currentVertexID = uint(gl_VertexID); 
    vec4 Pc = model * vec4(aPos, 1.0);
    gl_Position = projection * view * Pc;
    
    TexCoord = vec2(-1.0, -1.0);
    if(pValid[0] > 0.0 && Pc[2] > 0.0)
    {
        Pc[0] = Pc[0] / Pc[2];
        Pc[1] = Pc[1] / Pc[2];
        Pc[2] = 1.0;
        vec4 p = cam * Pc;
        if(p[0] >= 0 && p[0] < 1.0 && p[1] >= 0 && p[1] < 1.0)
            TexCoord = vec2(p[0], 1.0 - p[1]);
    }
}