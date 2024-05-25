#version 420 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoord;

out VS_OUT {
	vec2 textureCoordinate;
	vec3 normal; // World/Model space
} vs_out;

layout(std140, binding = 0) uniform CBPerFrame
{
    mat4 proj;
    mat4 proj_ortho;
    mat4 view;
    float width;
    float height;
} cbPerFrame;

void main() {
	gl_Position = cbPerFrame.proj * cbPerFrame.view * vec4(position, 1.0);
	vs_out.textureCoordinate = textureCoord;
	vs_out.normal = normal;
}

