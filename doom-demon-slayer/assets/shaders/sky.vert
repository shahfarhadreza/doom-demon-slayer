#version 420 core

layout (location = 0) in vec3 position;

out VS_OUT {
	vec3 textureCoordinate;
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

	mat3 viewRot = mat3(cbPerFrame.view);

	vec4 pos = cbPerFrame.proj * mat4(viewRot) * vec4(position, 1.0);
	//pos.z += 1; // Offset to avoid z-fighting 
	gl_Position = pos;
	vs_out.textureCoordinate = position;
}

