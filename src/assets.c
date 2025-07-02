#include "doom.h"

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define WHITESPACE " \t\n\r"
#define OBJ_FILENAME_LENGTH 500
#define MATERIAL_NAME_SIZE 255
#define OBJ_LINE_SIZE 500

char strequal(const char *s1, const char *s2) {
	if(strcmp(s1, s2) == 0)
		return 1;
	return 0;
}

char contains(const char *haystack, const char *needle) {
	if(strstr(haystack, needle) == NULL)
		return 0;
	return 1;
}

// Caller must free the returned string buffer
char* read_file_full(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);  /* same as rewind(f); */

        char* buf = malloc(fsize + 1);
        fread(buf, fsize, 1, file);
        fclose(file);

        buf[fsize] = 0;

        return buf;
    }
    return NULL;
}

int texture_load_cubemap(struct texture_t* tex, const char* filenames[]) {
    tex->texture_id = 0;
    tex->height = 0;
    tex->width = 0;

    glGenTextures(1, &tex->texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex->texture_id);

    int color_bit;
    for (unsigned int i = 0; i < 6; i++) {
        unsigned char *data = stbi_load(filenames[i], &tex->width, &tex->height, &color_bit, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, tex->width, tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else {
            printf("CubeMap texture failed to load at path: %s\n", filenames[i]);
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    printf("CubeMap Texture Loaded: %d X %d\n", tex->width, tex->height);
    return 1;
}

int texture_convert_dev(const char* filename) {
    int color_bit;
    int height = 0;
    int width = 0;

    unsigned char* data = stbi_load(filename, &width, &height, &color_bit, STBI_rgb_alpha);
    if (data) {
        int data_size = width * height * color_bit;

        FILE* f = fopen("./assets/textures/dev.dat", "wb");

        fwrite(&width, sizeof(int), 1, f);
        fwrite(&height, sizeof(int), 1, f);
        fwrite(&color_bit, sizeof(int), 1, f);

        fwrite(data, sizeof(char), data_size, f);

        fclose(f);

        // we don't need to keep the data in ram, it's a GPU resource
        stbi_image_free(data);

        return 1;
    }
    return 0;
}

int texture_load_dev(struct texture_t* tex, const char* filename) {

    FILE* f = fopen(filename, "rb");

    if (!f) {
        return 0;
    }

    int color_bit;

    tex->texture_id = 0;
    tex->height = 0;
    tex->width = 0;

    fread(&tex->width, sizeof(int), 1, f);
    fread(&tex->height, sizeof(int), 1, f);
    fread(&color_bit, sizeof(int), 1, f);

    assert(tex->width == 141);
    assert(tex->height == 10);
    assert(color_bit == 4);

    int data_size = tex->width * tex->height * color_bit;

    unsigned char* data = malloc(sizeof(char) * data_size);
    fread(data, sizeof(char), data_size, f);

    fclose(f);

    glGenTextures(1, &tex->texture_id);
    glBindTexture(GL_TEXTURE_2D, tex->texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,  GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);

    free(data);
}

// Caller must free the returned texture
int texture_load(struct texture_t* tex, const char* filename) {
    int color_bit;

    tex->texture_id = 0;
    tex->height = 0;
    tex->width = 0;

    unsigned char* data = stbi_load(filename, &tex->width, &tex->height, &color_bit, STBI_rgb_alpha);
    if (data) {
        int do_mipmap = 0;

        glGenTextures(1, &tex->texture_id);
        glBindTexture(GL_TEXTURE_2D, tex->texture_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,  GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (do_mipmap) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            // Also enable the mighty ANISOTROPY FILTER!
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->width, tex->height, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, data);

        if (do_mipmap) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        // we don't need to keep the data in ram, it's a GPU resource
        stbi_image_free(data);

        printf("Texture Loaded: %d X %d\n", tex->width, tex->height);

        return 1;
    }
    return 0;
}

void texture_free(struct texture_t* tex) {
    if (tex->texture_id != 0) {
        glDeleteTextures(1, &tex->texture_id);
    }
}

void obj_parse_vector(struct vec3_t* v) {
	v->x = atof( strtok(NULL, WHITESPACE));
	v->y = atof( strtok(NULL, WHITESPACE));
	v->z = atof( strtok(NULL, WHITESPACE));
}

void obj_parse_vector2(struct vec2_t* v) {
	v->x = atof( strtok(NULL, WHITESPACE));
	v->y = atof( strtok(NULL, WHITESPACE));
}

struct obj_face_t {
    int pos_index[3];
    int normal_index[3];
    int uv_index[3];
};

void obj_parse_face(struct obj_face_t* face) {
    char *temp_str;
	char *token;
	int vertex_count = 0;

	while( (token = strtok(NULL, WHITESPACE)) != NULL) {
	    if (vertex_count >= 3) {
            assert(0);
	    }
        face->uv_index[vertex_count] = 0;
        face->normal_index[vertex_count] = 0;
		face->pos_index[vertex_count] = atoi( token );

		//normal only
		if(contains(token, "//"))  {
			temp_str = strchr(token, '/');
			temp_str++;
			face->normal_index[vertex_count] = atoi( ++temp_str );
		}
		else if(contains(token, "/")) {
			temp_str = strchr(token, '/');
			face->uv_index[vertex_count] = atoi( ++temp_str );

			if(contains(temp_str, "/")) {
				temp_str = strchr(temp_str, '/');
				face->normal_index[vertex_count] = atoi( ++temp_str );
			}
		}
		vertex_count++;
	}
}

struct mesh_t* load_obj(const char* filename) {
    FILE* obj_file_stream;
	int current_material = -1;
	char *current_token = NULL;
	char current_line[OBJ_LINE_SIZE];
	int line_number = 0;
	// open scene
	obj_file_stream = fopen(filename, "r");
	if(obj_file_stream == 0) {
		fprintf(stderr, "Error reading file: %s\n", filename);
		return NULL;
	}

	struct mesh_t* mesh = malloc(sizeof(struct mesh_t));

	size_t pos_array_alloc = 100;
	size_t pos_array_count = 0;
	struct vec3_t* pos_array = malloc(sizeof(struct vec3_t) * pos_array_alloc);

	size_t norm_array_alloc = 100;
	size_t norm_array_count = 0;
	struct vec3_t* norm_array = malloc(sizeof(struct vec3_t) * norm_array_alloc);

	size_t uv_array_alloc = 100;
	size_t uv_array_count = 0;
	struct vec2_t* uv_array = malloc(sizeof(struct vec2_t) * uv_array_alloc);

	size_t vertex_array_alloc = 100;
	size_t vertex_array_count = 0;
	struct vertex_t* vertex_array = malloc(sizeof(struct vertex_t) * vertex_array_alloc);

	size_t index_array_alloc = 100;
	size_t index_array_count = 0;
	unsigned int* index_array = malloc(sizeof(unsigned int) * index_array_alloc);

	//parser loop
	while( fgets(current_line, OBJ_LINE_SIZE, obj_file_stream) ) {
		current_token = strtok( current_line, " \t\n\r");
		line_number++;

		//skip comments
		if( current_token == NULL || current_token[0] == '#')
			continue;
		//process vertex
		else if( strequal(current_token, "v") ) {
		    struct vec3_t pos;
		    obj_parse_vector(&pos);

		    //printf("pos %f, %f, %f\n", pos.x, pos.y, pos.z);

		    pos_array[pos_array_count] = pos;
		    pos_array_count++;
		    if (pos_array_count >= pos_array_alloc) {
                pos_array_alloc += 50;
                pos_array = realloc(pos_array, sizeof(struct vec3_t) * pos_array_alloc);
		    }
		}
		//process vertex normal
		else if( strequal(current_token, "vn") ) {
            struct vec3_t norm;
		    obj_parse_vector(&norm);

		    norm_array[norm_array_count] = norm;
		    norm_array_count++;
		    if (norm_array_count >= norm_array_alloc) {
                norm_array_alloc += 50;
                norm_array = realloc(norm_array, sizeof(struct vec3_t) * norm_array_alloc);
		    }
		}
		//process vertex texture
		else if( strequal(current_token, "vt") ) {
		    struct vec2_t uv;
		    obj_parse_vector2(&uv);

		    uv_array[uv_array_count] = uv;
		    uv_array_count++;
		    if (uv_array_count >= uv_array_alloc) {
                uv_array_alloc += 50;
                uv_array = realloc(uv_array, sizeof(struct vec2_t) * uv_array_alloc);
		    }
		}
		 //process face
		else if( strequal(current_token, "f") ){
            struct obj_face_t face;
			obj_parse_face(&face);

            // must be a triangle (3 indices)
            for(int f = 0;f < 3;f++) {
                int pos_idx = face.pos_index[f] - 1;
                int norm_idx = face.normal_index[f] - 1;
                int uv_idx = face.uv_index[f] - 1;

                struct vertex_t v;

                //printf("pos index %d\n", pos_idx);

                v.pos = pos_array[pos_idx];
                v.norm = norm_array[norm_idx];
                v.uv = uv_array[uv_idx];

                size_t vertex_index = -1;

                // TODO: Need faster algorithm
                for(int j = 0;j < vertex_array_count;j++) {
                    struct vertex_t* v_ptr = &vertex_array[j];

                    if (v_ptr->pos.x == v.pos.x && v_ptr->pos.y == v.pos.y && v_ptr->pos.z == v.pos.z &&
                        v_ptr->norm.x == v.norm.x && v_ptr->norm.y == v.norm.y && v_ptr->norm.z == v.norm.z) {
                        vertex_index = j;
                        //printf("found!\n");
                        break;
                    }
                }

                if (vertex_index == -1) {
                    vertex_array[vertex_array_count] = v;
                    vertex_index = vertex_array_count;
                    vertex_array_count++;
                    if (vertex_array_count >= vertex_array_alloc) {
                        vertex_array_alloc += 50;
                        vertex_array = realloc(vertex_array, sizeof(struct vertex_t) * vertex_array_alloc);
                    }
                }

                //printf("index %d\n", vertex_index);

                index_array[index_array_count] = vertex_index;
                index_array_count++;
                if (index_array_count >= index_array_alloc) {
                    index_array_alloc += 50;
                    index_array = realloc(index_array, sizeof(unsigned int) * index_array_alloc);
                }
            }

            //printf("face %d, %d, %d\n", face.pos_index[0], face.pos_index[1], face.pos_index[2]);
			//face->material_index = current_material;
			//list_add_item(&growable_data->face_list, face, NULL);
		}
		else if( strequal(current_token, "usemtl") ) // usemtl
		{
			//current_material = list_find(&growable_data->material_list, strtok(NULL, WHITESPACE));
		}

		else if( strequal(current_token, "mtllib") ) // mtllib
		{
		    /*
			strncpy(growable_data->material_filename, strtok(NULL, WHITESPACE), OBJ_FILENAME_LENGTH);
			obj_parse_mtl_file(growable_data->material_filename, &growable_data->material_list);
			continue;*/
		}
	}

	mesh->vertex_buffer = vertex_buffer_new(&vertex_array[0], vertex_array_count);
    mesh->index_buffer = index_buffer_new(&index_array[0], index_array_count);

	free(index_array);
	free(vertex_array);
	free(uv_array);
	free(norm_array);
	free(pos_array);

	fclose(obj_file_stream);
	return mesh;
}


