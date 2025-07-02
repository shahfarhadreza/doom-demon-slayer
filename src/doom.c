#include "doom.h"

struct cb_frame_data_t {
    struct mat4_t proj;
    struct mat4_t proj_ortho;
    struct mat4_t view;
    float width;
    float height;
};

struct cb_object_data_t {
    struct mat4_t world;
    float opacity;
};

#define DEMON_STATE_WALKING 1
#define DEMON_STATE_ATTACKING 2
#define DEMON_STATE_DYING 3

#define DEMON_TYPE_IMP 1
#define DEMON_TYPE_ARCH 2

struct demon_t {
    struct sprite3d_t* sprite;
    struct vec3_t position;
    float speed;
    float health;
    int state;
    int type;

    struct aabb_t world_aabb; // world space bounding box

    float animation_frame;
    int animation_reverse;
};

#define PICKUP_OBJECT_HEALTH 1
#define PICKUP_OBJECT_AMMO 2
#define PICKUP_OBJECT_ARMOR 3

struct pickup_object_t {
    // TODO: Replace with 3D mesh?
    struct sprite3d_t* sprite;

    struct vec3_t position;
    struct aabb_t world_aabb; // world space bounding box

    char type;
};

#define MAX_PICKUP_OBJECT 100

struct projectile_t {
    struct sprite3d_t* sprite;
    struct vec3_t origin; // where it starts
    struct vec3_t position; // current position
    struct vec3_t direction;
    int marked_for_removal;

    struct aabb_t world_aabb; // world space bounding box
};

#define MAX_PROJECTTILE 100

struct animated_effect_t {
    struct sprite3d_t* sprite;
    float frame;
    int max_frame;
    struct vec3_t position;
    int effect_type;
};

#define EFFECT_IMPACT 1
#define EFFECT_BLOOD 2
#define EFFECT_SPAWN 3

#define MAX_ANIMATED_EFFECTS 20

#define GAME_STATE_MENU 1
#define GAME_STATE_TUTORIAL 2
#define GAME_STATE_PLAYING 3
#define GAME_STATE_PAUSE 4
#define GAME_STATE_DEAD 5
#define GAME_STATE_SCORE 6

#define WEAPON_HAND 1
#define WEAPON_PISTOL 2

#define SCREEN_FLASH_RED 1
#define SCREEN_FLASH_GREEN 2

struct game_t {
    int width, height;
    GLFWwindow* window;
    int quit;

    struct cb_frame_data_t cb_frame_data;

    struct index_buffer_t* quad_ibuf;
    struct vertex_buffer_t* quad_vbuf;

    struct vertex_buffer_t* sky_vbuf;

    int state;
    float yaw;
    float pitch;

    // Dead State stuffs
    float text_blink_time;

    // Scenes/Objects
    struct mesh_t* scene;

    // Enemies
    struct demon_t* demons;
    size_t demon_count;
    size_t demon_alloc;

    // Pickup objects
    struct pickup_object_t* pickup_objects;
    size_t pickup_objects_count;

    // Effects
    struct animated_effect_t* animated_effects;
    size_t animated_effects_count;

    // Projectiles
    struct projectile_t* player_projectiles;
    size_t player_projectiles_count;

    // 3D Sprites
    struct sprite3d_t* sprite_imp;
    struct sprite3d_t* sprite_arch;
    struct sprite3d_t* sprite_projectile;
    struct sprite3d_t* sprite_explosion;
    struct sprite3d_t* sprite_spawn;
    struct sprite3d_t* sprite_blood;

    struct sprite3d_t* sprite_pickup_health;
    struct sprite3d_t* sprite_pickup_ammo;
    struct sprite3d_t* sprite_pickup_armor;
    struct sprite3d_t* sprite_pickup_pistol;

    int pickup_ammo_firsttime;

    // Textures
    struct texture_t texture;
    struct texture_t dev_texture;
    struct texture_t menu_texture;
    struct texture_t menu_play_texture;
    struct texture_t menu_quit_texture;
    struct texture_t menu_main_texture;
    struct texture_t menu_resume_texture;
    struct texture_t menu_skull_texture;
    struct texture_t cross_hair_texture;
    struct texture_t hud_texture_health_ammo;
    struct texture_t font_texture;
    struct texture_t skull_texture;
    struct texture_t red_texture;
    struct texture_t green_texture;
    struct texture_t dead_text_texture;
    struct texture_t total_kill_text_texture;
    struct texture_t paused_text_texture;

    struct texture_t sky_texture;

    // player
    struct texture_t player_hand_texture;
    struct texture_t player_pistol_texture;

    // Shaders
    GLuint sky_shader;
    GLuint lighting_shader;
    GLuint sprite3d_shader;
    GLuint hud_shader;

    // Constant Buffers
    struct constant_buffer_t* cb_object;

    // Screen Effect
    float screen_flash_opacity;
    int screen_flash_type;

    // player
    struct vec3_t cam_up;
    struct vec3_t cam_right;
    struct vec3_t cam_dir;
    struct vec3_t cam_pos;
    struct vec3_t player_pos;
    struct vec3_t player_velocity;
    float player_height;
    int player_health;
    int player_ammo;
    int player_armor;
    int player_state_attacking;
    int player_kill_count;
    int player_state_taking_damage;

    int player_weapon_type;

    float pistol_animation_time;

    float demon_spwan_rate;
    float dead_state_wait_time;
};

struct game_t* game = 0;

struct demon_t* game_spawn_demon(float x, float y, float z) {
    if (game->demon_count >= game->demon_alloc) {
        log_error("MAX DEMON LIMIT EXCEEDED");
        return 0;
    }
    struct demon_t* new_demon = &game->demons[game->demon_count];
    game->demon_count++;

    new_demon->health = 100;

    new_demon->position.x = x;
    new_demon->position.y = y;
    new_demon->position.z = z;

    int demon_exist = 0;
    for(int i = 0;i < game->demon_count;i++) {
        struct demon_t* demon = &game->demons[i];
        if (demon->type == DEMON_TYPE_ARCH) {
            demon_exist = 1;
        }
    }

    int rnd = get_rand(1, 10);

    if ((rnd % 2) && rnd > 6 && demon_exist == 0 && game->player_kill_count > 50) {
        new_demon->type = DEMON_TYPE_ARCH;
    } else {
        new_demon->type = DEMON_TYPE_IMP;
    }

    if (new_demon->type == DEMON_TYPE_ARCH) {
        new_demon->sprite = game->sprite_arch;
        new_demon->speed = get_randf(4.0f, 5.0f);
    } else {
        new_demon->sprite = game->sprite_imp;
        new_demon->speed = get_randf(3.0f, 4.5f);
    }

    new_demon->state = DEMON_STATE_WALKING;

    new_demon->animation_frame = 0.0f;
    new_demon->animation_reverse = 0;

    new_demon->world_aabb = new_demon->sprite->local_aabb;
    aabb_translate(&new_demon->world_aabb, &new_demon->position);

    return new_demon;
}

void render_world() {

    // Render sky
    glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

    glUseProgram(game->sky_shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, game->sky_texture.texture_id);

    glBindBuffer(GL_ARRAY_BUFFER, game->sky_vbuf->buffer_object);
    glBindVertexArray(game->sky_vbuf->array_object);
    glDrawArrays(GL_TRIANGLES, 0, game->sky_vbuf->count);

    glEnable(GL_DEPTH_TEST);

    // Render the level mesh
    glUseProgram(game->lighting_shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, game->texture.texture_id);

    glBindBuffer(GL_ARRAY_BUFFER, game->scene->vertex_buffer->buffer_object);
    glBindVertexArray(game->scene->vertex_buffer->array_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game->scene->index_buffer->buffer_object);

    glDrawElements(GL_TRIANGLES, game->scene->index_buffer->count, GL_UNSIGNED_INT, 0);

    // Render 3D sprites/billboards

    glUseProgram(game->sprite3d_shader);

    struct cb_object_data_t cb_object_data;

    //Demons
    for(int i = 0;i < game->demon_count;i++) {

        struct demon_t* demon = &game->demons[i];

        mat4_identity(&cb_object_data.world);
        mat4_translate(&cb_object_data.world, &demon->position);

        constant_buffer_update(game->cb_object, &cb_object_data);

        sprite3d_render(demon->sprite, demon->animation_frame);
    }

    // Pickups
    for(int i = 0;i < game->pickup_objects_count;i++) {

        struct pickup_object_t* obj = &game->pickup_objects[i];

        mat4_identity(&cb_object_data.world);
        mat4_translate(&cb_object_data.world, &obj->position);

        constant_buffer_update(game->cb_object, &cb_object_data);

        sprite3d_render(obj->sprite, 0);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render Effects
    for(int i = 0;i < game->animated_effects_count;i++) {
        struct animated_effect_t* explosion = &game->animated_effects[i];

        mat4_identity(&cb_object_data.world);
        mat4_translate(&cb_object_data.world, &explosion->position);

        constant_buffer_update(game->cb_object, &cb_object_data);

        sprite3d_render(explosion->sprite, explosion->frame);
    }


    // Render Projectiles
    for(int i = 0;i < game->player_projectiles_count;i++) {

        struct projectile_t* projectile = &game->player_projectiles[i];

        mat4_identity(&cb_object_data.world);
        mat4_translate(&cb_object_data.world, &projectile->position);

        constant_buffer_update(game->cb_object, &cb_object_data);

        sprite3d_render(projectile->sprite, 0);
    }

    glDisable(GL_BLEND);

}

void render_hud_quad_frame(struct texture_t* tex, float x, float y, float width, float height, int frame_w, int frame_h, float frame) {
    // Set a texture as well
    glBindTexture(GL_TEXTURE_2D, tex->texture_id);

    float tw = (float)frame_w / tex->width;
    float th = (float)frame_h / tex->height;
    // TODO: Only row based frames works
    int numPerRow = tex->width / frame_w;

    int frame_index = (int)frame;

    float tx = (frame_index % numPerRow) * tw;
    float ty = (frame_index / numPerRow + 1) * th;

    struct vertex_t quad_varray[] = {
        {.pos = {x,  y + height, 0.0f}, .uv = {tx, ty + th}},
        {.pos = {x,  y, 0.0f}, .uv = {tx, ty}},
        {.pos = {x + width,  y + height, 0.0f}, .uv = {tx + tw, ty + th}},
        {.pos = {x + width,  y, 0.0f}, .uv = {tx + tw, ty}},
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(struct vertex_t), quad_varray);

    glDrawElements(GL_TRIANGLES, game->quad_ibuf->count, GL_UNSIGNED_INT, 0);
}

void render_hud_quad(struct texture_t* tex, float x, float y, float width, float height) {
    // Set a texture as well
    glBindTexture(GL_TEXTURE_2D, tex->texture_id);

    struct vertex_t quad_varray[] = {
        {.pos = {x,  y + height, 0.0f}, .uv = {0.0f, 1.0f}},
        {.pos = {x,  y, 0.0f}, .uv = {0.0f, 0.0f}},
        {.pos = {x + width,  y + height, 0.0f}, .uv = {1.0f, 1.0f}},
        {.pos = {x + width,  y, 0.0f}, .uv = {1.0f, 0.0f}}
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(struct vertex_t), quad_varray);

    glDrawElements(GL_TRIANGLES, game->quad_ibuf->count, GL_UNSIGNED_INT, 0);
}

void render_digit(struct texture_t* tex, float x, float y, float width, float height, int number) {
    // Set a texture as well
    glBindTexture(GL_TEXTURE_2D, tex->texture_id);

    int frame_w = 8;
    int frame_h = 7;

    float tw = (float)frame_w / tex->width;
    float th = (float)frame_h / tex->height;
    // TODO: Only row based frames works
    int numPerRow = tex->width / frame_w;

    assert(number >= 0 && number <= 9);

    int frame_index = number;

    float tx = (frame_index % numPerRow) * tw;
    float ty = (frame_index / numPerRow + 1) * th;

    struct vertex_t quad_varray[] = {
        {.pos = {x,  y + height, 0.0f}, .uv = {tx, ty + th}},
        {.pos = {x,  y, 0.0f}, .uv = {tx, ty}},
        {.pos = {x + width,  y + height, 0.0f}, .uv = {tx + tw, ty + th}},
        {.pos = {x + width,  y, 0.0f}, .uv = {tx + tw, ty}},
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(struct vertex_t), quad_varray);

    glDrawElements(GL_TRIANGLES, game->quad_ibuf->count, GL_UNSIGNED_INT, 0);
}

void render_number(struct texture_t* tex, float x, float y, float width, float height, int number) {
    const int base = 10;
    int n = number;
    int digits[10];
    int digit_count = 0;
    float nx = x;
    if (number == 0) {
        render_digit(tex, nx, y, width, height, 0);
        return;
    }
    while(n != 0) {
        int digit = n % base;
        digits[digit_count] = digit;
        digit_count++;
        n = n / base;
    }
    for(int i = digit_count - 1; i >= 0;--i) {
        render_digit(tex, nx, y, width, height, digits[i]);
        nx += width + 1;
    }
}

float weapon_bob_timer = 0.0f;
float weapon_bob_speed = 15.0f;
float weapon_bob_amount = 15.0f;

float weapon_y = 0.0f;

void render_player() {
    float center_x = game->width / 2.0f;
    float center_y = game->height / 2.0f;

    float weapon_width = 230.0f;
    float weapon_height = 310.0f;

    if (game->player_weapon_type == WEAPON_HAND) {
        weapon_width = 260 * 2.5f;
        weapon_height = game->player_hand_texture.height * 2.5f;
    }

    float weapon_x = center_x - (weapon_width / 2.0f) - 30;

    weapon_y = (game->height - weapon_height);
    weapon_y += (sinf(weapon_bob_timer) * weapon_bob_amount) + weapon_bob_amount;
    //weapon_x += (cosf(weapon_bob_timer) * weapon_bob_amount) + weapon_bob_amount;

    if (game->player_weapon_type == WEAPON_HAND) {

        render_hud_quad_frame(&game->player_hand_texture, weapon_x, weapon_y,
                          weapon_width, weapon_height, 260, 77, game->pistol_animation_time);

    } else if (game->player_weapon_type == WEAPON_PISTOL) {
        render_hud_quad_frame(&game->player_pistol_texture, weapon_x, weapon_y,
                        weapon_width, weapon_height, 78, 103, game->pistol_animation_time);
    }
}

const float text_big_scale = 5.0f;

struct menu_item_t {
    float x, y;
    float width, height;
    struct texture_t* tex;
    int highlight;
};

#define MENU_COUNT 2
struct menu_item_t menus[MENU_COUNT];

void render_menu_items() {
    for(int i = 0;i < MENU_COUNT;i++) {
        struct menu_item_t* menu = &menus[i];
        render_hud_quad(menu->tex, menu->x, menu->y, menu->width, menu->height);

        if (menu->highlight) {
            float skull_scale = 0.6f;
            float skull_y = menu->y - 3;
            float skull_w = 36 * skull_scale;
            render_hud_quad(&game->menu_skull_texture, menu->x - skull_w, skull_y, skull_w, 50* skull_scale);
            render_hud_quad(&game->menu_skull_texture, (menu->x + menu->width), skull_y, skull_w, 50* skull_scale);
        }
    }
}

void render_menu_paused() {
    float center_x = game->width / 2.0f;
    float center_y = game->height / 2.0f;

    float text_width = game->paused_text_texture.width * text_big_scale;
    float text_height = game->paused_text_texture.height * text_big_scale;
    render_hud_quad(&game->paused_text_texture, center_x - (text_width/2), center_y - (text_height/2), text_width, text_height);

    render_menu_items();
}

void render_hud() {
    glUseProgram(game->hud_shader);
    glActiveTexture(GL_TEXTURE0);

    struct cb_object_data_t cb_object_data;

    cb_object_data.opacity = 1.0f;
    constant_buffer_update(game->cb_object, &cb_object_data);

    glBindBuffer(GL_ARRAY_BUFFER, game->quad_vbuf->buffer_object);
    glBindVertexArray(game->quad_vbuf->array_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game->quad_ibuf->buffer_object);

    float center_x = game->width / 2.0f;
    float center_y = game->height / 2.0f;

    if (game->state == GAME_STATE_DEAD) {
        float text_width = game->dead_text_texture.width * text_big_scale;
        float text_height = game->dead_text_texture.height * text_big_scale;
        if (game->text_blink_time > 0.3 && game->text_blink_time < 0.8) {
            render_hud_quad(&game->dead_text_texture, center_x - (text_width/2), center_y - (text_height/2), text_width, text_height);
        }
    } else if (game->state == GAME_STATE_PAUSE) {
        render_menu_paused();
    }

    // Draw Skull/Kill count
    render_number(&game->font_texture, 120, 50, 30, 35, game->player_kill_count);
    render_hud_quad(&game->skull_texture, 40, 30, 80, 60);

    if (game->state == GAME_STATE_PLAYING || game->state == GAME_STATE_PAUSE) {

        if (game->state == GAME_STATE_PLAYING) {
            if (game->player_weapon_type != WEAPON_HAND) {
                // Draw weapon crosshair
                float cross_width = 45.0f;
                float cross_height = 45.0f;
                float cross_x = center_x - (cross_width / 2.0f);
                float cross_y = center_y - (cross_height / 2.0f);

                render_hud_quad(&game->cross_hair_texture, cross_x, cross_y, cross_width, cross_height);
            }
        }

        // Draw the player hand/weapon
        render_player();
    }

    // Draw Ammo, Health

    float ammo_health_width = game->hud_texture_health_ammo.width * 2.5;
    float ammo_health_height = game->hud_texture_health_ammo.height * 2.5;

    float ammo_health_x = 50;

    render_number(&game->font_texture, ammo_health_x + 20, game->height - ammo_health_height + 10, 30, 30, game->player_ammo);

    render_number(&game->font_texture, ammo_health_x + 140, game->height - ammo_health_height + 10, 30, 30, game->player_health);

    render_hud_quad(&game->hud_texture_health_ammo, ammo_health_x,
                        game->height - ammo_health_height, ammo_health_width, ammo_health_height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    if (game->screen_flash_opacity > 0.0f) {
        cb_object_data.opacity = game->screen_flash_opacity;
        constant_buffer_update(game->cb_object, &cb_object_data);

        if (game->screen_flash_type == SCREEN_FLASH_RED) {
            render_hud_quad(&game->red_texture, 0, 0, game->width, game->height);
        } else if (game->screen_flash_type == SCREEN_FLASH_GREEN) {
            render_hud_quad(&game->green_texture, 0, 0, game->width, game->height);
        }
    }

    glDisable(GL_BLEND);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void render_score() {
    glUseProgram(game->hud_shader);
    glActiveTexture(GL_TEXTURE0);

    struct cb_object_data_t cb_object_data;

    cb_object_data.opacity = 1.0f;
    constant_buffer_update(game->cb_object, &cb_object_data);

    glBindBuffer(GL_ARRAY_BUFFER, game->quad_vbuf->buffer_object);
    glBindVertexArray(game->quad_vbuf->array_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game->quad_ibuf->buffer_object);

    float center_x = game->width / 2.0f;
    float center_y = game->height / 2.0f;

    render_menu_items();

    // Draw Skull/Kill count
    float text_width = 84 * 5;
    float text_height = 10 * 5;
    float text_y = center_y - 120;
    render_hud_quad(&game->total_kill_text_texture, center_x - (text_width/2), text_y, text_width, text_height);

    float kill_text_width = 40;
    float kill_text_x = center_x - (kill_text_width/2);
    float kill_text_y = text_y + 80;
    render_number(&game->font_texture, kill_text_x, kill_text_y, kill_text_width, 45, game->player_kill_count);

    float skull_width = 80;
    float skull_height = 60;
    render_hud_quad(&game->skull_texture, kill_text_x - skull_width - 10, kill_text_y - 10, skull_width, skull_height);

    render_hud_quad(&game->menu_texture, 0, 0, game->width, game->height);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void render_menu_main() {
    glUseProgram(game->hud_shader);
    glActiveTexture(GL_TEXTURE0);

    struct cb_object_data_t cb_object_data;

    cb_object_data.opacity = 1.0f;
    constant_buffer_update(game->cb_object, &cb_object_data);

    glBindBuffer(GL_ARRAY_BUFFER, game->quad_vbuf->buffer_object);
    glBindVertexArray(game->quad_vbuf->array_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game->quad_ibuf->buffer_object);

    const float center_x = game->width / 2.0f;
    const float center_y = game->height / 2.0f;

    float h = game->dev_texture.height * 1.5;

    render_hud_quad(&game->dev_texture, 5, game->height - h - 5, game->dev_texture.width * 1.5, h);

    render_menu_items();

    render_hud_quad(&game->menu_texture, 0, 0, game->width, game->height);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void game_menu_items_update(float dt) {
    for(int i = 0;i < MENU_COUNT;i++) {
        struct menu_item_t* menu = &menus[i];
        menu->highlight = 0;
    }

    double mouse_x, mouse_y;
    glfwGetCursorPos(game->window, &mouse_x, &mouse_y);

    for(int i = 0;i < MENU_COUNT;i++) {
        struct menu_item_t* menu = &menus[i];
        float right = menu->x + menu->width;
        float bottom = menu->y + menu->height;

        if (mouse_x >= menu->x && mouse_x <= right && mouse_y >= menu->y && mouse_y <= bottom) {
            menu->highlight = 1;
            break;
        }
    }
}

void game_menu_update(float dt) {

    const float center_x = game->width / 2.0f;
    const float center_y = game->height / 2.0f;

    const float menu_scale = 3.0f;
    const float menu_gap = 50.0f;

    float menu_play_width = game->menu_play_texture.width * menu_scale;
    float menu_play_height = game->menu_play_texture.height * menu_scale;
    float menu_play_x = center_x - (menu_play_width/2);
    float menu_play_y = center_y - menu_gap;

    menus[0].x = menu_play_x;
    menus[0].y = menu_play_y;
    menus[0].width = menu_play_width;
    menus[0].height = menu_play_height;
    menus[0].tex = &game->menu_play_texture;

    float menu_quit_width = game->menu_quit_texture.width * menu_scale;
    float menu_quit_height = game->menu_quit_texture.height * menu_scale;
    float menu_quit_x = center_x - (menu_quit_width/2);
    float menu_quit_y = center_y;

    menus[1].x = menu_quit_x;
    menus[1].y = menu_quit_y;
    menus[1].width = menu_quit_width;
    menus[1].height = menu_quit_height;
    menus[1].tex = &game->menu_quit_texture;

    game_menu_items_update(dt);

    if (glfwGetKey(game->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        game->quit = 1;
    }
}

void process_mouse_button(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_RELEASE) {
            if (game->state == GAME_STATE_MENU) {
                if (menus[0].highlight) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    game->state = GAME_STATE_PLAYING;
                    game_reset();
                } else if (menus[1].highlight) {
                    game->quit = 1;
                }
            } else if (game->state == GAME_STATE_PAUSE) {
                if (menus[0].highlight) {
                    game->state = GAME_STATE_PLAYING;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                } else if (menus[1].highlight) {
                    game->state = GAME_STATE_MENU;
                }
            } else if (game->state == GAME_STATE_SCORE) {
                if (menus[0].highlight) {
                    game->quit = 1;
                } else if (menus[1].highlight) {
                    game->state = GAME_STATE_MENU;
                }
            }
        }
    }
}

struct animated_effect_t* game_impact_effect_add(struct vec3_t* p, int type) {
    struct animated_effect_t* explosion = &game->animated_effects[game->animated_effects_count];
    game->animated_effects_count++;

    if (type == EFFECT_IMPACT) {
        explosion->sprite = game->sprite_explosion;
        explosion->max_frame = 5;
    } else if (type == EFFECT_SPAWN) {
        explosion->sprite = game->sprite_spawn;
        explosion->max_frame = 10;
    } else if (type == EFFECT_BLOOD) {
        explosion->sprite = game->sprite_blood;
        explosion->max_frame = 3;
    } else {
        assert(0);
    }

    explosion->frame = 0;
    explosion->position = *p;
    explosion->effect_type = type;

    return explosion;
}

void game_impact_effect_remove(int index) {
    int count = game->animated_effects_count - 1;
    for(int i = index; i < count; i++) {
        game->animated_effects[i] = game->animated_effects[i + 1];
    }
    game->animated_effects_count--;
}

void game_pickup_add(struct vec3_t* p, char type) {
    struct pickup_object_t* obj = &game->pickup_objects[game->pickup_objects_count];
    game->pickup_objects_count++;

    obj->type = type;
    obj->position = *p;

    if (type == PICKUP_OBJECT_HEALTH) {
        obj->sprite = game->sprite_pickup_health;
    } else if (type == PICKUP_OBJECT_AMMO) {
        if (game->pickup_ammo_firsttime) {
            obj->sprite = game->sprite_pickup_pistol;
            game->pickup_ammo_firsttime = 0;
        } else {
            obj->sprite = game->sprite_pickup_ammo;
        }
    } else if (type == PICKUP_OBJECT_ARMOR) {
        obj->sprite = game->sprite_pickup_armor;
    } else {
        assert(0);
    }

    obj->world_aabb = obj->sprite->local_aabb;
    aabb_translate(&obj->world_aabb, &obj->position);
}

void game_pickup_remove(int index) {
    int count = game->pickup_objects_count - 1;
    for(int i = index; i < count; i++) {
        game->pickup_objects[i] = game->pickup_objects[i + 1];
    }
    game->pickup_objects_count--;
}

void game_projectile_remove(int index) {
    int count = game->player_projectiles_count - 1;
    for(int i = index; i < count; i++) {
        game->player_projectiles[i] = game->player_projectiles[i + 1];
    }
    game->player_projectiles_count--;
}

void game_demon_remove(int index) {
    int count = game->demon_count - 1;
    for(int i = index; i < count; i++) {
        game->demons[i] = game->demons[i + 1];
    }
    game->demon_count--;
}

float camera_impact = 0.0f;

void player_attack_punch(float dt) {
    for(int i = 0;i < game->demon_count;i++) {
        struct demon_t* demon = &game->demons[i];

        // Simple distance based attack collision with the player
        float player_dist = vec3_distance(&demon->position, &game->player_pos);

        if (player_dist < 4) {
            if (demon->type == DEMON_TYPE_IMP) {
                demon->health -= get_randf(8, 20);
            } else {
                demon->health -= get_randf(1, 5);
            }

            struct vec3_t punch_pos = vec3(demon->position.x,
                                           demon->position.y + ((demon->sprite->scale_h) / 2),
                                           demon->position.z);

            struct animated_effect_t* effect = game_impact_effect_add(&punch_pos, EFFECT_BLOOD);
            //effect->frame = 3;

            // Apply backward force
            struct vec3_t dir = vec3_sub(&game->player_pos, &demon->position);
            vec3_normalize(&dir);
            struct vec3_t velocity = vec3_mulf(&dir, 10.0f * dt);
            demon->position = vec3_sub(&demon->position, &velocity);
        }
    }
}

void player_switch_weapon(int weapon) {
    game->player_weapon_type = weapon;
}

void player_attack(float dt) {
    if (game->player_weapon_type == WEAPON_HAND) {
        game->player_state_attacking = 1; // triggers the animation
        game->pistol_animation_time = 0.0f;
        camera_impact = 0.7f;
    } else {
        if (game->player_ammo > 0) {
            struct projectile_t* projectile = &game->player_projectiles[game->player_projectiles_count];
            game->player_projectiles_count++;

            projectile->sprite = game->sprite_projectile;

            projectile->origin.x = game->cam_pos.x;
            projectile->origin.y = game->cam_pos.y - 0.5f;
            projectile->origin.z = game->cam_pos.z;

            float offset_x = 0.0f;
            float offset_y = -0.1f;
            float offset_z = 1.0f; // a slight forward so it looks coming out from the gun

            struct vec3_t side = vec3_mulf(&game->cam_right, offset_x);
            struct vec3_t up = vec3_mulf(&game->cam_up, offset_y);
            struct vec3_t forward = vec3_mulf(&game->cam_dir, offset_z);

            projectile->origin = vec3_add(&projectile->origin, &side);
            projectile->origin = vec3_add(&projectile->origin, &up);
            projectile->origin = vec3_add(&projectile->origin, &forward);

            projectile->position = projectile->origin;
            projectile->direction = game->cam_dir;
            projectile->marked_for_removal = 0;

            projectile->world_aabb = projectile->sprite->local_aabb;
            aabb_translate(&projectile->world_aabb, &projectile->position);

            game->player_ammo--;

            if (game->player_ammo <= 0) {
                player_switch_weapon(WEAPON_HAND);
            }

            game->player_state_attacking = 1; // triggers the animation
            game->pistol_animation_time = 0.0f;

            camera_impact = 0.7f;
        }
    }
}

int mouse_down = 0;

void game_play_update_mouse_input(float dt) {

    int state = glfwGetMouseButton(game->window, GLFW_MOUSE_BUTTON_LEFT);

    if (state == GLFW_PRESS) {
        if (!mouse_down) {
            //printf("shoot idx %d\n", game->player_projectiles_count);
            player_attack(dt);
            mouse_down = 1;
        }
    } else {
        mouse_down = 0;
    }
}

void player_update_movement(float dt) {
    struct vec3_t moveDir = {0, 0, 0};

    float move_speed = 10.0f;

    struct vec3_t old_pos = game->player_pos;

    if (glfwGetKey(game->window, GLFW_KEY_W) == GLFW_PRESS) {
        struct vec3_t vec = {1.0, 0.0, 1.0};
        struct vec3_t new_dir = vec3_mul(&vec, &game->cam_dir);
        moveDir = vec3_add(&moveDir, &new_dir);
    }
    if (glfwGetKey(game->window, GLFW_KEY_S) == GLFW_PRESS) {
        struct vec3_t vec = {-1.0, 0.0, -1.0};
        struct vec3_t new_dir = vec3_mul(&vec, &game->cam_dir);
        moveDir = vec3_add(&moveDir, &new_dir);
    }

    if (glfwGetKey(game->window, GLFW_KEY_A) == GLFW_PRESS) {
        struct vec3_t vec = {-1.0, 0.0, -1.0};

        struct vec3_t new_dir = vec3_mul(&vec, &game->cam_right);
        moveDir = vec3_add(&moveDir, &new_dir);
    }
    if (glfwGetKey(game->window, GLFW_KEY_D) == GLFW_PRESS) {
        struct vec3_t vec = {1.0, 0.0, 1.0};

        struct vec3_t new_dir = vec3_mul(&vec, &game->cam_right);
        moveDir = vec3_add(&moveDir, &new_dir);
    }

    game->player_velocity.x = moveDir.x * move_speed * dt;
    game->player_velocity.z = moveDir.z * move_speed * dt;

    game->player_pos.x += game->player_velocity.x;
    game->player_pos.z += game->player_velocity.z;

    if (game->player_pos.x > 28 || game->player_pos.x < -28) {
        game->player_pos.x = old_pos.x;
    }

    if (game->player_pos.z > 28 || game->player_pos.z < -28) {
        game->player_pos.z = old_pos.z;
    }
}

float demon_spwan_time = 0.0f;

int firstMouse = 1;

struct vec2_t mouse_pos_last = {0, 0};
float mouse_sensitivity = 0.1f;

struct vec3_t world_up = {0, 1, 0};

void game_spawn_enimies(float dt) {
    if (demon_spwan_time > game->demon_spwan_rate) {
        demon_spwan_time = 0.0f;
        // the effect will spawn the demon at a specific frame
        float spawn_x = get_randf(-20, 20);
        float spawn_z = get_randf(-20, 20);
        float spawn_y = 0.0f;
        struct vec3_t spawn_pos = vec3(spawn_x, spawn_y, spawn_z);
        game_impact_effect_add(&spawn_pos, EFFECT_SPAWN);
    } else {
        demon_spwan_time += dt;
    }

    if (game->player_health <= 0) {
        game->state = GAME_STATE_DEAD;
    }
}

void game_increase_spawn_rate() {
    // Increase spawn rate as we kill ^_^
    game->demon_spwan_rate -= get_randf(0.03f, 0.1f);
    // Also, CAP
    if (game->demon_spwan_rate <= 1.5f) {
        game->demon_spwan_rate = 1.5f;
    }
}

void game_play_update_camera(float dt) {
    double mouse_x, mouse_y;
    glfwGetCursorPos(game->window, &mouse_x, &mouse_y);

    struct vec2_t mouse_pos = {.x = mouse_x, .y = mouse_y};

    if (firstMouse) {
        mouse_pos_last = mouse_pos;
        firstMouse = 0;
    }

    struct vec2_t delta = vec2_sub(&mouse_pos, &mouse_pos_last);
    mouse_pos_last = mouse_pos;

    // multiplying with the mouse sensitivity
    delta.x *= mouse_sensitivity;// * dt;
    delta.y *= mouse_sensitivity;// * dt;

    game->yaw += delta.x;
    game->pitch -= delta.y;

    float pitch_limit = 88.0f;

    // checking if pitch is within limit
    if (game->pitch > pitch_limit)
        game->pitch = pitch_limit;
    if (game->pitch < -pitch_limit)
        game->pitch = -pitch_limit;

    game->cam_dir.x = cos(math_deg_to_rad(game->yaw)) * cos(math_deg_to_rad(game->pitch));
    game->cam_dir.y = sin(math_deg_to_rad(game->pitch + camera_impact));
    game->cam_dir.z = sin(math_deg_to_rad(game->yaw)) * cos(math_deg_to_rad(game->pitch));
    vec3_normalize(&game->cam_dir);

    game->cam_right = vec3_cross(&game->cam_dir, &world_up);
    vec3_normalize(&game->cam_right);

    game->cam_up  = vec3_cross(&game->cam_right, &game->cam_dir);
    vec3_normalize(&game->cam_up);

    mat4_perspective(&game->cb_frame_data.proj, math_deg_to_rad(75.0f + camera_impact), (float)game->width / (float)game->height, 0.01f, 1000.0f);

    if (camera_impact > 0.0f) {
        camera_impact -= dt * 5.0f;
    }
}

void game_play_update_demons(float dt) {
    int demon_remove_index = -1;
    for(int i = 0;i < game->demon_count;i++) {
        struct demon_t* demon = &game->demons[i];

        // Make it chase after player!
        if (demon->state == DEMON_STATE_WALKING) {
            struct vec3_t dir = vec3_sub(&game->player_pos, &demon->position);
            vec3_normalize(&dir);

            struct vec3_t velocity = vec3_mulf(&dir, demon->speed * dt);

            demon->position = vec3_add(&demon->position, &velocity);

            demon->world_aabb = demon->sprite->local_aabb;
            aabb_translate(&demon->world_aabb, &demon->position);
        }

        // Simple distance based attack collision with the player

        if (demon->health > 0) {
            float player_dist = vec3_distance(&demon->position, &game->player_pos);

            if (player_dist < 3.0) {
                demon->state = DEMON_STATE_ATTACKING;
            } else {
                demon->state = DEMON_STATE_WALKING;
            }

            for(int j = 0;j < game->player_projectiles_count;j++) {

                struct projectile_t* projectile = &game->player_projectiles[j];

                if (projectile->marked_for_removal) {
                    continue;
                }

                //printf("max %f, %f, %f\n", projectile->world_aabb.max.x, projectile->world_aabb.max.y, projectile->world_aabb.max.z);

                if (aabb_intersect(&demon->world_aabb, &projectile->world_aabb)) {

                    if (demon->type == DEMON_TYPE_IMP) {
                        demon->health -= get_randf(18, 50);
                    } else {
                        demon->health -= get_randf(2, 8);
                    }
                    game_impact_effect_add(&projectile->position, EFFECT_IMPACT);

                    projectile->marked_for_removal = 1;
                    break;
                }
            }
        }

        if (demon->state == DEMON_STATE_WALKING) {
            if (demon->type == DEMON_TYPE_IMP) {
                if (demon->animation_frame < 3) {
                    demon->animation_frame += dt * 8.0f;
                } else {
                    demon->animation_frame = 0;
                }
            } else {
                if (demon->animation_reverse) {
                    demon->animation_frame -= dt * 10.0f;

                    if (demon->animation_frame < 0) {
                        demon->animation_frame += dt * 10.0f;
                        demon->animation_reverse = 0;
                    }
                } else {
                    demon->animation_frame += dt * 10.0f;

                    if (demon->animation_frame > 3) {
                        demon->animation_frame -= dt * 10.0f;
                        demon->animation_reverse = 1;
                    }
                }
            }
        } else if (demon->state == DEMON_STATE_ATTACKING) {
            if ( demon->animation_frame < 4) {
                 demon->animation_frame = 4;
            }
            if (demon->animation_frame < 7) {
                demon->animation_frame += dt * 15.0f;
            } else {
                // Perform the action at the end of the animation
                //if (!game->player_state_taking_damage) {
                    game->player_health -= get_rand(10, 20);
                    game->player_health = fmax(game->player_health, 0);
                    game->player_state_taking_damage = 1;

                    game->screen_flash_opacity = 0.7f;
                    game->screen_flash_type = SCREEN_FLASH_RED;
                //}
                demon->animation_frame = 4;
            }
        } else if (demon->state == DEMON_STATE_DYING) {
            if ( demon->animation_frame < 7) {
                 demon->animation_frame = 7;
            }
            if (demon->animation_frame < 10) {
                demon->animation_frame += dt * 12.0f;
            } else {
                demon->animation_frame = 10;
                demon_remove_index = i;

                // randomly spawn a pickup object
                int odds = get_rand(1, 5);

                if ((odds % 2) == 0) {
                    char pickup_type = get_rand(PICKUP_OBJECT_HEALTH, PICKUP_OBJECT_AMMO);
                    game_pickup_add(&demon->position, pickup_type);
                }
            }
        }

        if (demon->health <= 0) {
            if (demon->state != DEMON_STATE_DYING) {
                game->player_kill_count++;
                demon->state = DEMON_STATE_DYING;

                // Apply backward force
                struct vec3_t dir = vec3_sub(&game->player_pos, &demon->position);
                vec3_normalize(&dir);
                struct vec3_t velocity = vec3_mulf(&dir, 30.0f * dt);
                demon->position = vec3_sub(&demon->position, &velocity);
            }
        }
    }

    if (demon_remove_index >= 0) {
        game_increase_spawn_rate();
        game_demon_remove(demon_remove_index);
    }
}

// updates while in playing state
void game_play_update(float dt) {

    // do the camera rotation
    game_play_update_camera(dt);

    game_play_update_mouse_input(dt);

    player_update_movement(dt);

    float abs_x = fabs(game->player_velocity.x);
    float abs_z = fabs(game->player_velocity.z);

    float camera_bob = 0.0f;

    // if player is walking
    if (abs_x > 0.0f || abs_z > 0.0f) {
        weapon_bob_timer += dt * weapon_bob_speed;

        camera_bob = sinf(weapon_bob_timer) * 0.1f;
    } else {
        // slow breathe movement
        weapon_bob_timer += dt * (weapon_bob_speed * 0.07f);
    }

    // The camera stays with the player
    game->cam_pos = vec3(game->player_pos.x, game->player_pos.y + game->player_height + 0.1f + camera_bob, game->player_pos.z);

    // Lets not keep increasing it forever
    if (weapon_bob_timer > 6.2f) {
        weapon_bob_timer = 0.0f;
    }

    // Update player/weapon animation
    if (game->player_state_attacking) {
        if (game->player_weapon_type == WEAPON_HAND) {
            if (game->pistol_animation_time < 3) {
                game->pistol_animation_time += dt * 25.0f;
            } else {
                // Punching animation done, do damage
                player_attack_punch(dt);
                // Reset
                game->pistol_animation_time = 0.0f;
                game->player_state_attacking = 0;
            }
        } else {
            if (game->pistol_animation_time < 4) {
                game->pistol_animation_time += dt * 25.0f;
            } else {
                // Shooting done
                game->pistol_animation_time = 0.0f;
                game->player_state_attacking = 0;
            }
        }
    }

    if (game->screen_flash_opacity >= 0.0f) {
        game->screen_flash_opacity -= dt * 1.5f;
    } else {
        game->screen_flash_opacity = 0.0f;
    }

    // Update Effects
    int explosion_remove_index = -1;
    for(int i = 0;i < game->animated_effects_count;i++) {
        struct animated_effect_t* explosion = &game->animated_effects[i];

        if (explosion->frame < (explosion->max_frame-1)) {
            explosion->frame += dt * 25.0f;
        } else {
            if (explosion->effect_type == EFFECT_SPAWN) {
                game_spawn_demon(explosion->position.x, explosion->position.y, explosion->position.z);
            }
            explosion_remove_index = i;
        }
    }

    if (explosion_remove_index >= 0) {
        game_impact_effect_remove(explosion_remove_index);
    }

    // Update Projectiles
    for(int i = 0;i < game->player_projectiles_count;i++) {
        struct projectile_t* projectile = &game->player_projectiles[i];

        float speed = 50.0f;

        // Simply travel through the given direction
        struct vec3_t accelaration = vec3_mulf(&projectile->direction, speed * dt);
        projectile->position = vec3_add(&projectile->position, &accelaration);

        float distance = vec3_distance(&projectile->origin, &projectile->position);

        if (distance > 100.0f) {
            projectile->marked_for_removal = 1;
        }

        projectile->world_aabb = projectile->sprite->local_aabb;
        aabb_translate(&projectile->world_aabb, &projectile->position);
    }

    // Check for removal
    int remove_index = -1;
    for(int i = 0;i < game->player_projectiles_count;i++) {
        struct projectile_t* projectile = &game->player_projectiles[i];
        if (projectile->marked_for_removal) {
            remove_index = i;
            break;
        }
    }
    if (remove_index >= 0) {
        game_projectile_remove(remove_index);
    }

    // Update Demons
    game_play_update_demons(dt);

    // Update Pickup objects and check for interactions
    int pickup_remove_index = -1;
    for(int i = 0;i < game->pickup_objects_count;i++) {
        struct pickup_object_t* pickup = &game->pickup_objects[i];

        float dist = vec3_distance(&pickup->position, &game->player_pos);

        if (dist < 2) {
            // pickup the object and remove it
            if (pickup->type == PICKUP_OBJECT_HEALTH) {
                game->player_health += get_rand(10, 30);
                game->player_health = fmin(game->player_health, 100);

                game->screen_flash_opacity = 0.7f;
                game->screen_flash_type = SCREEN_FLASH_GREEN;

            } else if (pickup->type == PICKUP_OBJECT_AMMO) {
                game->player_ammo += get_rand(10, 25);
                player_switch_weapon(WEAPON_PISTOL);

                game->screen_flash_opacity = 0.7f;
                game->screen_flash_type = SCREEN_FLASH_GREEN;
            } else if (pickup->type == PICKUP_OBJECT_ARMOR) {
                game->player_armor += get_rand(20, 50);
                game->player_armor = fmin(game->player_armor, 100);

                game->screen_flash_opacity = 0.7f;
                game->screen_flash_type = SCREEN_FLASH_GREEN;
            }
            pickup_remove_index = i;
            break;
        }
    }

    if (pickup_remove_index >= 0) {
        game_pickup_remove(pickup_remove_index);
    }

    // YAY!
    game_spawn_enimies(dt);
}

void process_key_press(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (game->state == GAME_STATE_PLAYING) {
            game->state = GAME_STATE_PAUSE;
            firstMouse = 1;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else if (game->state == GAME_STATE_PAUSE) {
            game->state = GAME_STATE_PLAYING;
            firstMouse = 1;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void game_pause_update(float dt) {
    const float center_x = game->width / 2.0f;
    const float center_y = game->height / 2.0f;

    const float menu_scale = 3.0f;
    const float menu_gap = 50.0f;

    float menu_play_width = game->menu_resume_texture.width * menu_scale;
    float menu_quit_width = game->menu_main_texture.width * menu_scale;

    float menu_play_height = game->menu_resume_texture.height * menu_scale;
    float menu_play_x = center_x - ((menu_play_width + menu_quit_width + menu_gap) / 2);
    float menu_play_y = center_y + menu_gap;

    menus[0].x = menu_play_x;
    menus[0].y = menu_play_y;
    menus[0].width = menu_play_width;
    menus[0].height = menu_play_height;
    menus[0].tex = &game->menu_resume_texture;

    float menu_quit_height = game->menu_main_texture.height * menu_scale;
    float menu_quit_x = menu_play_x + menu_play_width + menu_gap;
    float menu_quit_y = menu_play_y;

    menus[1].x = menu_quit_x;
    menus[1].y = menu_quit_y;
    menus[1].width = menu_quit_width;
    menus[1].height = menu_quit_height;
    menus[1].tex = &game->menu_main_texture;

    game_menu_items_update(dt);
}

void game_state_score_update(float dt) {
    const float center_x = game->width / 2.0f;
    const float center_y = game->height / 2.0f;

    const float menu_scale = 3.0f;
    const float menu_gap = 50.0f;

    float menu_play_width = game->menu_quit_texture.width * menu_scale;
    float menu_quit_width = game->menu_main_texture.width * menu_scale;

    float menu_play_height = game->menu_quit_texture.height * menu_scale;
    float menu_play_x = center_x - ((menu_play_width + menu_quit_width + menu_gap) / 2);
    float menu_play_y = center_y + menu_gap;

    menus[0].x = menu_play_x;
    menus[0].y = menu_play_y;
    menus[0].width = menu_play_width;
    menus[0].height = menu_play_height;
    menus[0].tex = &game->menu_quit_texture;

    float menu_quit_height = game->menu_main_texture.height * menu_scale;
    float menu_quit_x = menu_play_x + menu_play_width + menu_gap;
    float menu_quit_y = menu_play_y;

    menus[1].x = menu_quit_x;
    menus[1].y = menu_quit_y;
    menus[1].width = menu_quit_width;
    menus[1].height = menu_quit_height;
    menus[1].tex = &game->menu_main_texture;

    game_menu_items_update(dt);
}

void game_update(float dt) {
    if (game->state == GAME_STATE_PLAYING) {
        game_play_update(dt);
    } else if (game->state == GAME_STATE_PAUSE) {
        game_pause_update(dt);
    } else if (game->state == GAME_STATE_DEAD) {
        game->text_blink_time += dt * 1.1f;
        if (game->text_blink_time > 1.0f) {
            game->text_blink_time = 0.0f;
        }
        // Wait a bit and then switch to score board state
        game->dead_state_wait_time -= dt * 1.5f;
        if (game->dead_state_wait_time < 1.0f) {
            game->state = GAME_STATE_SCORE;
            glfwSetInputMode(game->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    } else if (game->state == GAME_STATE_MENU) {
        game_menu_update(dt);
    } else if (game->state == GAME_STATE_SCORE) {
        game_state_score_update(dt);
    }
}

void game_render() {
    if (game->state == GAME_STATE_MENU) {
        render_menu_main();
    } else if (game->state == GAME_STATE_TUTORIAL || game->state == GAME_STATE_PLAYING || game->state == GAME_STATE_PAUSE) {
        render_world();
        render_hud();
    } else if (game->state == GAME_STATE_DEAD) {
        render_world();
        render_hud();
    } else if (game->state == GAME_STATE_SCORE) {
        render_score();
    }
}

void log_error(const char* msg) {
    printf("ERROR: %s\n", msg);
}

void log_info(const char* msg) {
    printf("LOG: %s\n", msg);
}

void log_info2(const char* title, const char* msg) {
    printf("LOG: %s: %s\n", title, msg);
}

void process_mouse_move(GLFWwindow* window, double xPosition, double yPosition) {

}

float skybox_vertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

void game_update_projections(int width, int height) {
    // Setup scene/camera projections
    mat4_perspective(&game->cb_frame_data.proj, math_deg_to_rad(75.0f), (float)width / (float)height, 0.01f, 1000.0f);
    mat4_ortho(&game->cb_frame_data.proj_ortho, 0, width, height, 0, -2.1f, 10.0f);
}

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
    game_update_projections(width, height);
}

void game_reset() {
    game->player_projectiles_count = 0;
    game->animated_effects_count = 0;
    game->pickup_objects_count = 0;
    game->demon_count = 0;

    game->pickup_ammo_firsttime = 1;

    game->cam_dir = vec3(0, 0, 0);

    game->player_pos = vec3(5, 0, -10);
    game->player_height = 2.2f;
    game->player_health = 100;
    game->player_ammo = 0;
    game->player_armor = 0;
    game->pistol_animation_time = 0.0f;
    game->player_state_attacking = 0;
    game->player_state_taking_damage = 0;
    game->player_kill_count = 0;

    game->player_weapon_type = WEAPON_HAND;

    game->screen_flash_opacity = 0.0f;
    game->screen_flash_type = SCREEN_FLASH_RED;

    game->text_blink_time = 0.0f;

    game->demon_spwan_rate = 5.0f;
    game->dead_state_wait_time = 6;

    game->yaw = 100;
    game->pitch = 0;
}

int main() {
    game = malloc(sizeof(struct game_t));

    game->width = 1280;
    game->height = 720;
    game->quit = 0;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(game->width, game->height, "DOOM: DEMON SLAYER (Developed by Shah Farhad Reza)", 0, 0);
	if (window == 0) {
		log_error("WINDOW::CREATION");
		log_error("OpenGL 3.2 Required");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	game->window = window;

	glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
	glfwSetCursorPosCallback(window, process_mouse_move);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetMouseButtonCallback(window, process_mouse_button);
	glfwSetKeyCallback(window, process_key_press);

	// GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		log_error("Failed to initialize GLAD");
		return -1;
	}

	log_info2("OpenGL", glGetString(GL_VERSION));

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	struct vertex_t quad_varray[] = {
        {.pos = {0,  1, 0.0f}, .uv = {0.0f, 1.0f}},
        {.pos = {0,  0, 0.0f}, .uv = {0.0f, 0.0f}},
        {.pos = {1,  1, 0.0f}, .uv = {1.0f, 1.0f}},
        {.pos = {1,  0, 0.0f}, .uv = {1.0f, 0.0f}},
    };

	unsigned int quad_iarray[] = {
	    0, 1, 2,
        1, 3, 2
	};

	game->quad_vbuf = vertex_buffer_new(&quad_varray[0], 4);
	game->quad_ibuf = index_buffer_new(&quad_iarray[0], 6);

	// SkyBox

    struct vertex_t sky_varray[36];

    int index = 0;
    for(int i = 0;i < 108;i += 3) {
        float x = skybox_vertices[i];
        float y = skybox_vertices[i+1];
        float z = skybox_vertices[i+2];

        sky_varray[index].pos = vec3(x, y, z);
        index++;
    }

	game->sky_vbuf = vertex_buffer_new(&sky_varray[0], 36);

	// Load assets

	// Shaders
	game->sky_shader = glsl_shader_program_new(
                                "./assets/shaders/sky.vert",
                                    "./assets/shaders/sky.frag");

	game->lighting_shader = glsl_shader_program_new(
                                "./assets/shaders/textured_lighting.vert",
                                    "./assets/shaders/textured_lighting.frag");

    game->sprite3d_shader = glsl_shader_program_new(
                                "./assets/shaders/sprite3d.vert",
                                    "./assets/shaders/sprite3d.frag");

    game->hud_shader = glsl_shader_program_new(
                                "./assets/shaders/hud.vert",
                                    "./assets/shaders/hud.frag");

    //texture_convert_dev("./assets/textures/test.png");

    if (!texture_load_dev(&game->dev_texture, "./assets/textures/dev.dat")) {
        return -1;
    }

    // Textures
    texture_load(&game->texture, "./assets/textures/floor2.png");
    texture_load(&game->menu_texture, "./assets/textures/menu.jpg");
    texture_load(&game->menu_skull_texture, "./assets/textures/menu_skull.png");
    texture_load(&game->menu_play_texture, "./assets/textures/text_play.png");
    texture_load(&game->menu_resume_texture, "./assets/textures/text_resume.png");
    texture_load(&game->menu_main_texture, "./assets/textures/text_main_menu.png");
    texture_load(&game->menu_quit_texture, "./assets/textures/text_quit.png");
    texture_load(&game->cross_hair_texture, "./assets/textures/crosshair.png");
    texture_load(&game->hud_texture_health_ammo, "./assets/textures/health_ammo.png");
    texture_load(&game->font_texture, "./assets/textures/font.png");
    texture_load(&game->skull_texture, "./assets/textures/skull.png");
    texture_load(&game->red_texture, "./assets/textures/red.png");
    texture_load(&game->green_texture, "./assets/textures/green.png");
    texture_load(&game->dead_text_texture, "./assets/textures/you are dead.png");
    texture_load(&game->total_kill_text_texture, "./assets/textures/text_final_score.png");
    texture_load(&game->paused_text_texture, "./assets/textures/text_paused.png");

    texture_load(&game->player_hand_texture, "./assets/textures/hand.png");
    texture_load(&game->player_pistol_texture, "./assets/textures/pistol.png");

    const char* sky_textures[] = {
        "./assets/textures/sky/right.png",
        "./assets/textures/sky/left.png",
        "./assets/textures/sky/top.png",
        "./assets/textures/sky/bottom.png",
        "./assets/textures/sky/back.png",
        "./assets/textures/sky/front.png",
    };

    texture_load_cubemap(&game->sky_texture, sky_textures);

    // Scene
    game->scene = load_obj("./assets/scenes/main.obj");

    // Sprites
    game->sprite_imp = sprite3d_new("./assets/textures/imp.png", 2.5, 3.2);
    game->sprite_imp->frame_w = 40;
    game->sprite_imp->frame_h = 57;

    game->sprite_arch = sprite3d_new("./assets/textures/arch.png", 2.5, 3.5);
    game->sprite_arch->frame_w = 40;
    game->sprite_arch->frame_h = 56;

    game->sprite_projectile = sprite3d_new("./assets/textures/projectile.png", 0.5, 0.5);

    game->sprite_explosion = sprite3d_new("./assets/textures/impact_explosion.png", 1, 1);
    game->sprite_explosion->frame_w = 50;
    game->sprite_explosion->frame_h = 47;

    game->sprite_blood = sprite3d_new("./assets/textures/impact_blood.png", 1, 1);
    game->sprite_blood->frame_w = 45;
    game->sprite_blood->frame_h = 40;

    game->sprite_spawn = sprite3d_new("./assets/textures/spawn_effect.png", 3, 3);
    game->sprite_spawn->frame_w = 40;
    game->sprite_spawn->frame_h = 37;

    game->sprite_pickup_health = sprite3d_new("./assets/textures/health.png", 1.3, 1);
    game->sprite_pickup_ammo = sprite3d_new("./assets/textures/ammo.png", 1.3, 1);
    game->sprite_pickup_armor = sprite3d_new("./assets/textures/armor.png", 1.3, 1.25);
    game->sprite_pickup_pistol = sprite3d_new("./assets/textures/pistol_pickup.png", 1.8, 0.9);

    // init demon objects
    game->demon_alloc = 100; // max demons
	game->demons = malloc(sizeof(struct demon_t) * game->demon_alloc);

	// init projectiles
	game->player_projectiles = malloc(sizeof(struct projectile_t) * MAX_PROJECTTILE);
	// Effects
    game->animated_effects = malloc(sizeof(struct animated_effect_t) * MAX_ANIMATED_EFFECTS);
    // Pickup Objects
    game->pickup_objects = malloc(sizeof(struct pickup_object_t) * MAX_PICKUP_OBJECT);

    game_update_projections(game->width, game->height);

    struct constant_buffer_t* cb_frame = constant_buffer_new(sizeof(struct cb_frame_data_t));
    game->cb_object = constant_buffer_new(sizeof(struct cb_object_data_t));

    // We can bind them immediately now
    // bind frame constant buffer at '0' index
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cb_frame->buffer_object);
    // bind object constant buffer at '1' index
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, game->cb_object->buffer_object);

    game_reset();

    game->state = GAME_STATE_MENU;

    double last_time = glfwGetTime();

	// game loop
	while (!glfwWindowShouldClose(window)) {

        double current_time = glfwGetTime();
		// delta time ()
		double dt = current_time - last_time;
		last_time = current_time;

		if (game->quit) {
            break;
		}

		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		    printf("player %f, %f, %f\n", game->player_pos.x, game->player_pos.y, game->player_pos.z);
		}

        glfwGetWindowSize(window, &game->width, &game->height);
        glViewport(0, 0, game->width, game->height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        game_update(dt);

		// Update the frame constant buffer
        struct vec3_t cam_lookat = vec3_add(&game->cam_pos, &game->cam_dir);

        mat4_identity(&game->cb_frame_data.view);
        mat4_lookAt(&game->cb_frame_data.view, &game->cam_pos, &cam_lookat, &game->cam_up);

        game->cb_frame_data.width = game->width;
        game->cb_frame_data.height = game->height;

        constant_buffer_update(cb_frame, &game->cb_frame_data);

        game_render();

        glfwSwapBuffers(window);
	    glfwPollEvents();
	}

	log_info("Cleaning up...");

	free(game->pickup_objects);
	free(game->animated_effects);
	free(game->player_projectiles);
	free(game->demons);

	texture_free(&game->sky_texture);
	texture_free(&game->paused_text_texture);
	texture_free(&game->total_kill_text_texture);
	texture_free(&game->dead_text_texture);
	texture_free(&game->red_texture);
	texture_free(&game->green_texture);
	texture_free(&game->texture);
	texture_free(&game->font_texture);
	texture_free(&game->skull_texture);
	texture_free(&game->cross_hair_texture);
	texture_free(&game->hud_texture_health_ammo);
	texture_free(&game->player_pistol_texture);
	texture_free(&game->player_hand_texture);
	texture_free(&game->menu_play_texture);
	texture_free(&game->menu_quit_texture);
	texture_free(&game->menu_main_texture);
	texture_free(&game->menu_resume_texture);
	texture_free(&game->menu_skull_texture);
	texture_free(&game->menu_texture);

	sprite3d_delete(game->sprite_spawn);
	sprite3d_delete(game->sprite_explosion);
	sprite3d_delete(game->sprite_projectile);
	sprite3d_delete(game->sprite_arch);
	sprite3d_delete(game->sprite_imp);

	sprite3d_delete(game->sprite_pickup_pistol);
	sprite3d_delete(game->sprite_pickup_armor);
	sprite3d_delete(game->sprite_pickup_ammo);
	sprite3d_delete(game->sprite_pickup_health);

	if (game->scene) {
        index_buffer_delete(game->scene->index_buffer);
        vertex_buffer_delete(game->scene->vertex_buffer);
        free(game->scene);
	}

    index_buffer_delete(game->quad_ibuf);
    vertex_buffer_delete(game->quad_vbuf);

    vertex_buffer_delete(game->sky_vbuf);

    constant_buffer_delete(game->cb_object);
	constant_buffer_delete(cb_frame);

    glDeleteProgram(game->hud_shader);
    glDeleteProgram(game->sprite3d_shader);
    glDeleteProgram(game->lighting_shader);
    glDeleteProgram(game->sky_shader);

	glfwDestroyWindow(window);
	glfwTerminate();

	free(game);
	return 0;
}



