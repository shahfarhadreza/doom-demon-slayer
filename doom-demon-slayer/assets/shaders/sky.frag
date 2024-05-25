#version 420 core

layout(binding = 0) uniform samplerCube texture_sampler;

layout(location = 0) out vec4 fragColor;

in VS_OUT {
    vec3 textureCoordinate;
} fs_in;

void main() {
    fragColor = texture(texture_sampler, fs_in.textureCoordinate);
}


