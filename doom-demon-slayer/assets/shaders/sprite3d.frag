#version 420 core

layout(binding = 0) uniform sampler2D texture_sampler;

layout(location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 textureCoordinate;
    vec3 normal; // World/Model space
} fs_in;

void main() {
    
    vec4 albedo_color = texture(texture_sampler, fs_in.textureCoordinate).rgba;

    if (albedo_color.a < 0.01)
	    discard;

    fragColor = albedo_color;
}


