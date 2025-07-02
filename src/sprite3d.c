#include "doom.h"

struct sprite3d_t* sprite3d_new(const char* filename, float scale_w, float scale_h) {
    struct sprite3d_t* sprite = malloc(sizeof(struct sprite3d_t));
    texture_load(&sprite->texture, filename);

    sprite->scale_w = scale_w;
    sprite->scale_h = scale_h;

    sprite->frame_w = sprite->texture.width;
    sprite->frame_h = sprite->texture.height;

    float half_w = scale_w / 2.0f;

    // very small z value added only for a proper bounding box intersection
    struct vertex_t quad_varray[] = {
        {.pos = {-half_w,  scale_h, 0.0f}, .uv = {0.0f, -1.0f}},
        {.pos = {-half_w,  0, 0.0f}, .uv = {0.0f, 0.0f}},
        {.pos = {half_w,  scale_h, 0.0f}, .uv = {1.0f, -1.0f}},
        {.pos = {half_w,  0, 0.0f}, .uv = {1.0f, 0.0f}},
    };

	unsigned int quad_iarray[] = {
	    0, 1, 2,
        1, 3, 2
	};

	sprite->quad_vbuf = vertex_buffer_new(&quad_varray[0], 4);
	sprite->quad_ibuf = index_buffer_new(&quad_iarray[0], 6);

	// build axis-aligned bounding box
	aabb_init(&sprite->local_aabb);
	for(int i = 0;i < 4;i++) {
	    struct vec3_t pos_bb = quad_varray[i].pos;
	    // z value added only for a proper bounding box intersection
        if (i == 0 || i == 1) {
            pos_bb.z = -half_w;
        } else {
            pos_bb.z = half_w;
        }
        aabb_extend(&sprite->local_aabb, &pos_bb);
	}

    return sprite;
};

void sprite3d_delete(struct sprite3d_t* sprite) {
    index_buffer_delete(sprite->quad_ibuf);
    vertex_buffer_delete(sprite->quad_vbuf);
    texture_free(&sprite->texture);
    free(sprite);
}

void sprite3d_render(struct sprite3d_t* sprite, float frame) {

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sprite->texture.texture_id);

    glBindBuffer(GL_ARRAY_BUFFER, sprite->quad_vbuf->buffer_object);
    glBindVertexArray(sprite->quad_vbuf->array_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite->quad_ibuf->buffer_object);

    // update VBO for frames/animation
    float half_w = sprite->scale_w / 2.0f;

    float tw = (float)sprite->frame_w / sprite->texture.width;
    float th = (float)sprite->frame_h / sprite->texture.height;
    // TODO: Only row based frames works
    int numPerRow = sprite->texture.width / sprite->frame_w;

    int frame_index = (int)frame;

    float tx = (frame_index % numPerRow) * tw;
    float ty = (frame_index / numPerRow + 1) * th;

    struct vertex_t quad_varray[] = {
        {.pos = {-half_w,  sprite->scale_h, 0.0f},  .uv = {tx, ty - th}},
        {.pos = {-half_w,  0, 0.0f},                .uv = {tx, ty}},
        {.pos = {half_w,  sprite->scale_h, 0.0f},   .uv = {tx + tw, ty - th}},
        {.pos = {half_w,  0, 0.0f},                 .uv = {tx + tw, ty}},
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(struct vertex_t), quad_varray);

    glDrawElements(GL_TRIANGLES, sprite->quad_ibuf->count, GL_UNSIGNED_INT, 0);
}

