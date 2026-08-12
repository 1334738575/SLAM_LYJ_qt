#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 pValid;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 cameraSize;
uniform float cameraParameters[8];
uniform int cameraModel;

out vec2 TexCoord;

void main(){
    uint currentVertexID = uint(gl_VertexID); 
    vec4 Pw = vec4(aPos, 1.0);
    gl_Position = projection * view * Pw;
    
    vec4 Pc = model * Pw;
    TexCoord = vec2(-1.0, -1.0);
    if(pValid[0] > 0.0 && Pc[2] > 0.0 && cameraSize.x > 0.0 && cameraSize.y > 0.0)
    {
        vec2 normalized = Pc.xy / Pc.z;
        if(cameraModel == 1)
        {
            float radius = length(normalized);
            if(radius > 1e-12)
            {
                float theta = atan(radius);
                float theta2 = theta * theta;
                float theta4 = theta2 * theta2;
                float theta6 = theta4 * theta2;
                float theta8 = theta4 * theta4;
                float thetaDistorted = theta * (1.0 + cameraParameters[4] * theta2
                    + cameraParameters[5] * theta4 + cameraParameters[6] * theta6
                    + cameraParameters[7] * theta8);
                normalized *= thetaDistorted / radius;
            }
            else
            {
                normalized = vec2(0.0);
            }
        }
        vec2 pixel = vec2(cameraParameters[0], cameraParameters[1]) * normalized
            + vec2(cameraParameters[2], cameraParameters[3]);
        vec2 uv = pixel / cameraSize;
        if(uv.x >= 0.0 && uv.x < 1.0 && uv.y >= 0.0 && uv.y < 1.0)
            TexCoord = vec2(uv.x, 1.0 - uv.y);
    }
}
