#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct vec2_t {
    float x, y;
};

struct vec3_t {
    float x, y, z;
};

struct vec4_t {
    float x, y, z, w;
};

struct mat4_t {
	union {
		float m[4][4];
		float M[16];
	};
};

struct aabb_t {
    struct vec3_t min, max;
};

struct vertex_t {
    struct vec3_t pos;
    struct vec3_t norm;
    struct vec2_t uv;
};

struct vertex_buffer_t {
    GLuint array_object;
    GLuint buffer_object;
    size_t count;
};

struct index_buffer_t {
    GLuint buffer_object;
    size_t count;
};

struct constant_buffer_t {
    GLuint buffer_object;
    size_t size;
};

struct texture_t {
    GLuint texture_id;
    size_t width, height;
};

// Every individual mesh that have unique mat/texture
struct mesh_t {
    struct vertex_buffer_t* vertex_buffer;
    struct index_buffer_t* index_buffer;
    struct texture_t texture;
};

struct sprite3d_t {
    struct texture_t texture;
    struct index_buffer_t* quad_ibuf;
    struct vertex_buffer_t* quad_vbuf;

    float scale_w, scale_h;

    int frame_w, frame_h;

    struct aabb_t local_aabb;
};

#define PI 3.14159265359f
#define DEGTORAD (PI / 180.0f)

float math_deg_to_rad(float rad);

void log_error(const char* msg);
void log_info(const char* msg);
void log_info2(const char* title, const char* msg);

char* read_file_full(const char* filename);

int texture_load(struct texture_t* tex, const char* filename);
int texture_convert_dev(const char* filename);
int texture_load_dev(struct texture_t* tex, const char* filename);
int texture_load_cubemap(struct texture_t* tex, const char* filenames[]);
void texture_free(struct texture_t* tex);

GLuint glsl_shader_program_new(const char* vert_filename, const char* frag_filename);

struct vertex_buffer_t* vertex_buffer_new(struct vertex_t* data, size_t count);
void vertex_buffer_delete(struct vertex_buffer_t* buf);

struct index_buffer_t* index_buffer_new(unsigned int* data, size_t count);
void index_buffer_delete(struct index_buffer_t* buf);

struct constant_buffer_t* constant_buffer_new(size_t sizeinBytes);
void constant_buffer_delete(struct constant_buffer_t* buf);
void constant_buffer_update(struct constant_buffer_t* buf, void* data);

struct sprite3d_t* sprite3d_new(const char* filename, float scale_w, float scale_h);
void sprite3d_delete(struct sprite3d_t* sprite);
void sprite3d_render(struct sprite3d_t* sprite, float frame);

void mat4_identity(struct mat4_t* mat);
void mat4_perspective(struct mat4_t* mat, float fov, float aspect, float zNear, float zFar);
void mat4_ortho(struct mat4_t* mat, float left, float right, float bottom, float top, float zNear, float zFar);
void mat4_lookAt(struct mat4_t* mat, struct vec3_t* eye, struct vec3_t* center, struct vec3_t* up);
void mat4_set_translation(struct mat4_t* mat, float x, float y, float z);

struct mesh_t* load_obj(const char* filename);

void mat4_inverse(struct mat4_t* mat, struct mat4_t* inv);
void mat4_translate(struct mat4_t* mat, struct vec3_t* v);

struct vec2_t vec2_sub(struct vec2_t* a, struct vec2_t* b);

struct vec3_t vec3(float x, float y, float z);
struct vec3_t vec3_add(struct vec3_t* a, struct vec3_t* b);
struct vec3_t vec3_sub(struct vec3_t* a, struct vec3_t* b);
struct vec3_t vec3_mul(struct vec3_t* a, struct vec3_t* b);
struct vec3_t vec3_mulf(struct vec3_t* a, float f);
struct vec3_t vec3_cross(struct vec3_t* a, struct vec3_t* b);
float vec3_distance(struct vec3_t* a, struct vec3_t* b);
void vec3_normalize(struct vec3_t* v);


void aabb_init(struct aabb_t* aabb);
void aabb_extend(struct aabb_t* aabb, struct vec3_t* p);
void aabb_translate(struct aabb_t* aabb, struct vec3_t* v);
int aabb_intersect(struct aabb_t* a, struct aabb_t* b);

int get_rand(int min, int max);
float get_randf(float a, float b);

