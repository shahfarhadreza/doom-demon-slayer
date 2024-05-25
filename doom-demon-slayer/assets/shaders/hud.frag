#version 420 core

layout(binding = 0) uniform sampler2D texture_sampler;

layout(location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 textureCoordinate;
} fs_in;

layout(std140, binding = 1) uniform CBPerObject
{
    mat4 world;
    float opacity;
} cbPerObject;

void main() {
    
    vec4 albedo_color = texture(texture_sampler, fs_in.textureCoordinate).rgba;

    if (albedo_color.a < 0.01)
	    discard;

    fragColor = vec4(albedo_color.rgb, albedo_color.a * cbPerObject.opacity);
}


