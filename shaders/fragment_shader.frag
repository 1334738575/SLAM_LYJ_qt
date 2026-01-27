#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ourTexture; // 纹理采样器

void main(){
    // FragColor = vec4(0.8, 0.3, 0.2, 1.0);
    if(TexCoord.x < 0 || TexCoord.y < 0)
        FragColor = vec4(0.f, 0.f, 0.f, 1.0f);
    else
        FragColor = texture(ourTexture, TexCoord);
}