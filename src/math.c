#include "doom.h"

float math_deg_to_rad(float rad) {
    return rad * DEGTORAD;
}

int get_rand(int min, int max) {
    return rand()%(max-min + 1) + min;
}

float get_randf(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

struct vec2_t vec2_sub(struct vec2_t* a, struct vec2_t* b) {
    struct vec2_t out;
    out.x = a->x - b->x;
    out.y = a->y - b->y;
    return out;
}

struct vec3_t vec3(float x, float y, float z) {
    struct vec3_t v = {x, y, z};
    return v;
};

void mat4_identity(struct mat4_t* mat) {
    memset(mat->M, 0, 16*sizeof(float));

    mat->m[0][0] = 1.0f;
    mat->m[1][1] = 1.0f;
    mat->m[2][2] = 1.0f;
    mat->m[3][3] = 1.0f;
}

void mat4_perspective(struct mat4_t* mat, float fov, float aspect, float zNear, float zFar) {
    memset(mat->M, 0, 16*sizeof(float));

    float tanHalfFovy = tan(fov / 2.0f);
    mat->m[0][0] = 1.0f / (aspect * tanHalfFovy);
    mat->m[1][1] = 1.0f / (tanHalfFovy);
    mat->m[2][2] = - (zFar + zNear) / (zFar - zNear);
    mat->m[2][3] = - 1.0f;
    mat->m[3][2] = - (2.0f * zFar * zNear) / (zFar - zNear);
}

void mat4_ortho(struct mat4_t* mat, float left, float right, float bottom, float top, float zNear, float zFar) {
    mat4_identity(mat);

    mat->m[0][0] = (2) / (right - left);
    mat->m[1][1] = (2) / (top - bottom);
    mat->m[2][2] = - (2) / (zFar - zNear);

    mat->m[3][0] = - (right + left) / (right - left);
    mat->m[3][1] = - (top + bottom) / (top - bottom);

    mat->m[3][2] = - (zFar + zNear) / (zFar - zNear);
}

struct vec3_t vec3_add(struct vec3_t* a, struct vec3_t* b) {
    struct vec3_t out;
    out.x = a->x + b->x;
    out.y = a->y + b->y;
    out.z = a->z + b->z;
    return out;
}

struct vec3_t vec3_sub(struct vec3_t* a, struct vec3_t* b) {
    struct vec3_t out;
    out.x = a->x - b->x;
    out.y = a->y - b->y;
    out.z = a->z - b->z;
    return out;
}

struct vec3_t vec3_mul(struct vec3_t* a, struct vec3_t* b) {
    struct vec3_t out;
    out.x = a->x * b->x;
    out.y = a->y * b->y;
    out.z = a->z * b->z;
    return out;
}

struct vec3_t vec3_mulf(struct vec3_t* a, float f) {
    struct vec3_t out;
    out.x = a->x * f;
    out.y = a->y * f;
    out.z = a->z * f;
    return out;
}

struct vec3_t vec3_cross(struct vec3_t* a, struct vec3_t* b) {
    struct vec3_t out;
    out.x = a->y * b->z - a->z * b->y;
	out.y = a->z * b->x - a->x * b->z;
	out.z = a->x * b->y - a->y * b->x;
    return out;
}

float vec3_dot(struct vec3_t* a, struct vec3_t* b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

float vec3_length(struct vec3_t* v) {
    return sqrt((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

float vec3_distance(struct vec3_t* a, struct vec3_t* b) {
    struct vec3_t d = vec3_sub(a, b);
    return vec3_length(&d);
}

void vec3_normalize(struct vec3_t* v) {
    float length_of_v = vec3_length(v);
    v->x = v->x / length_of_v;
    v->y = v->y / length_of_v;
    v->z = v->z / length_of_v;
}

// lookat view-matrix
void mat4_lookAt(struct mat4_t* mat, struct vec3_t* eye, struct vec3_t* center, struct vec3_t* up) {
    struct vec3_t f = vec3_sub(center, eye);
    vec3_normalize(&f);

    struct vec3_t u = *up;
    vec3_normalize(&u);

    struct vec3_t s = vec3_cross(&f, &u);
    vec3_normalize(&s);
    u = vec3_cross(&s, &f);

    mat->m[0][0] = s.x;
    mat->m[1][0] = s.y;
    mat->m[2][0] = s.z;
    mat->m[3][0] = 0.0f;

    mat->m[0][1] = u.x;
    mat->m[1][1] = u.y;
    mat->m[2][1] = u.z;
    mat->m[3][1] = 0.0f;

    mat->m[0][2] =-f.x;
    mat->m[1][2] =-f.y;
    mat->m[2][2] =-f.z;
    mat->m[3][2] = 0.0f;

    mat->m[3][0] =-vec3_dot(&s, eye);
    mat->m[3][1] =-vec3_dot(&u, eye);
    mat->m[3][2] = vec3_dot(&f, eye);
    mat->m[3][3] = 1.0f;
}

void mat4_translate(struct mat4_t* mat, struct vec3_t* v) {
    mat->m[3][0] = v->x;
    mat->m[3][1] = v->y;
    mat->m[3][2] = v->z;
}

void mat4_inverse(struct mat4_t* mat, struct mat4_t* inv)
{
    float m00 = mat->m[0][0], m01 = mat->m[0][1], m02 = mat->m[0][2], m03 = mat->m[0][3];
    float m10 = mat->m[1][0], m11 = mat->m[1][1], m12 = mat->m[1][2], m13 = mat->m[1][3];
    float m20 = mat->m[2][0], m21 = mat->m[2][1], m22 = mat->m[2][2], m23 = mat->m[2][3];
    float m30 = mat->m[3][0], m31 = mat->m[3][1], m32 = mat->m[3][2], m33 = mat->m[3][3];

    float v0 = m20 * m31 - m21 * m30;
    float v1 = m20 * m32 - m22 * m30;
    float v2 = m20 * m33 - m23 * m30;
    float v3 = m21 * m32 - m22 * m31;
    float v4 = m21 * m33 - m23 * m31;
    float v5 = m22 * m33 - m23 * m32;

    float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
    float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
    float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
    float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

    float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

    float d00 = t00 * invDet;
    float d10 = t10 * invDet;
    float d20 = t20 * invDet;
    float d30 = t30 * invDet;

    float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m10 * m31 - m11 * m30;
    v1 = m10 * m32 - m12 * m30;
    v2 = m10 * m33 - m13 * m30;
    v3 = m11 * m32 - m12 * m31;
    v4 = m11 * m33 - m13 * m31;
    v5 = m12 * m33 - m13 * m32;

    float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m21 * m10 - m20 * m11;
    v1 = m22 * m10 - m20 * m12;
    v2 = m23 * m10 - m20 * m13;
    v3 = m22 * m11 - m21 * m12;
    v4 = m23 * m11 - m21 * m13;
    v5 = m23 * m12 - m22 * m13;

    float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    inv->M[0] = d00;
    inv->M[1] = d01;
    inv->M[2] = d02;
    inv->M[3] = d03;
    inv->M[4] = d10;
    inv->M[5] = d11;
    inv->M[6] = d12;
    inv->M[7] = d13,
    inv->M[8] = d20;
    inv->M[9] = d21;
    inv->M[10] = d22;
    inv->M[11] = d23;
    inv->M[12] = d30;
    inv->M[13] = d31;
    inv->M[14] = d32;
    inv->M[15] = d33;
}

struct vec3_t vec3_min(struct vec3_t* v1, struct vec3_t* v2) {
    return vec3(fmin(v1->x, v2->x), fmin(v1->y, v2->y), fmin(v1->z, v2->z));
}

struct vec3_t vec3_max(struct vec3_t* v1, struct vec3_t* v2) {
    return vec3(fmax(v1->x, v2->x), fmax(v1->y, v2->y), fmax(v1->z, v2->z));
}

void aabb_init(struct aabb_t* aabb) {
    aabb->min = vec3(1, 1, 1);
    aabb->max = vec3(-1, -1, -1);
}

int aabb_is_null(struct aabb_t* aabb) {
    return aabb->min.x > aabb->max.x || aabb->min.y > aabb->max.y || aabb->min.z > aabb->max.z;
}

void aabb_extend(struct aabb_t* aabb, struct vec3_t* p) {
    if (!aabb_is_null(aabb)) {
        aabb->min = vec3_min(p, &aabb->min);
        aabb->max = vec3_max(p, &aabb->max);
    } else {
        aabb->min = *p;
        aabb->max = *p;
    }
}

void aabb_translate(struct aabb_t* aabb, struct vec3_t* v) {
    if (!aabb_is_null(aabb)) {
        aabb->min = vec3_add(&aabb->min, v);
        aabb->max = vec3_add(&aabb->max, v);
    }
}
/*
void AABB::transform(const glm::mat4& m) {
    if (!isNull()) {
        glm::vec4 mn(mMin, 1);
        glm::vec4 mx(mMax, 1);

        mn = m * mn;
        mx = m * mx;

        mMin = {mn.x, mn.y, mn.z};
        mMax = {mx.x, mx.y, mx.z};
    }
}*/

int aabb_intersect(struct aabb_t* a, struct aabb_t* b) {
    if (aabb_is_null(a) || aabb_is_null(b)) {
        return 0;
    }

    if ((a->max.x < b->min.x) || (a->min.x > b->max.x) ||
        (a->max.y < b->min.y) || (a->min.y > b->max.y) ||
        (a->max.z < b->min.z) || (a->min.z > b->max.z))
    {
        return 0;
    }

    if ((a->min.x <= b->min.x) && (a->max.x >= b->max.x) &&
        (a->min.y <= b->min.y) && (a->max.y >= b->max.y) &&
        (a->min.z <= b->min.z) && (a->max.z >= b->max.z))
    {
        // Inside
        return 1;
    }
    return 1;
}










