#version 330 core
in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out uvec4 faceId;

uniform sampler2D ourTexture; // 纹理采样器

void main(){
    // FragColor = vec4(0.8, 0.3, 0.2, 1.0);
    if(TexCoord.x < 0 || TexCoord.y < 0)
        FragColor = vec4(05.f, 05.f, 05.f, 1.0f);
    else
        FragColor = texture(ourTexture, TexCoord);
    faceId = uvec4(
        ((gl_PrimitiveID >> 24) & 0xFF) + 1,
        (gl_PrimitiveID >> 16) & 0xFF,
        (gl_PrimitiveID >> 8)  & 0xFF,
        gl_PrimitiveID         & 0xFF
    );
}