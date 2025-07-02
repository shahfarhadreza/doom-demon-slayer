#include "doom.h"

GLuint glsl_shader_program_new(const char* vert_filename, const char* frag_filename) {

    log_info("Reading vertex shader file...");
    const GLchar* vert_code = read_file_full(vert_filename);

    if (!vert_code) {
        log_error("Failed to read vertex shader file.");
        return 0;
    }

    GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);

    log_info("Compiling vertex shader...");
    glShaderSource(vert_shader_id, 1, &vert_code, NULL);
	glCompileShader(vert_shader_id);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Check Shader
	log_info("Checking vertex shader...");
	glGetShaderiv(vert_shader_id, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(vert_shader_id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
        char* VertexShaderErrorMessage = malloc(sizeof(char) * (InfoLogLength + 1));
		glGetShaderInfoLog(vert_shader_id, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("VSHADER ERROR: %s\n", &VertexShaderErrorMessage[0]);
		free(VertexShaderErrorMessage);
		glDeleteShader(vert_shader_id);
		return 0;
	}

	free(vert_code);

	log_info("Reading fragment shader file...");
	const GLchar* frag_code = read_file_full(frag_filename);

	if (!frag_code) {
        log_error("Failed to read fragment shader file.");
        return 0;
    }

	GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    log_info("Compiling fragment shader...");
    glShaderSource(frag_shader_id, 1, &frag_code, NULL);
	glCompileShader(frag_shader_id);

	// Check Shader
	log_info("Checking fragment shader...");
	glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(frag_shader_id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
        char* VertexShaderErrorMessage = malloc(sizeof(char) * (InfoLogLength + 1));
		glGetShaderInfoLog(frag_shader_id, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("FSHADER ERROR: %s\n", &VertexShaderErrorMessage[0]);
		free(VertexShaderErrorMessage);
		glDeleteShader(frag_shader_id);
		return 0;
	}

	free(frag_code);

	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vert_shader_id);
	glAttachShader(program_id, frag_shader_id);
	glLinkProgram(program_id);

	// Check the program
	glGetProgramiv(program_id, GL_LINK_STATUS, &Result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		//std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		//glGetProgramInfoLog(mProgramId, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		//printf("%s\n", &ProgramErrorMessage[0]);
		//assert(0);
	}

	glDetachShader(program_id, vert_shader_id);
	glDetachShader(program_id, frag_shader_id);

	return program_id;
}

struct vertex_buffer_t* vertex_buffer_new(struct vertex_t* data, size_t count) {
    struct vertex_buffer_t* buf = malloc(sizeof(struct vertex_buffer_t));

    buf->count = count;

    size_t nSize = sizeof(struct vertex_t);

    glGenVertexArrays(1, &buf->array_object);
	glBindVertexArray(buf->array_object);

    glGenBuffers(1, &buf->buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, buf->buffer_object);

    glBufferData(GL_ARRAY_BUFFER, count * nSize, data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	int nOffset = 0;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, nSize, 0);
	nOffset += 3;

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 3;

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 2;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

    return buf;
}

void vertex_buffer_delete(struct vertex_buffer_t* buf) {
    glDeleteBuffers(1, &buf->buffer_object);
    glDeleteVertexArrays(1, &buf->array_object);
    free(buf);
}

struct index_buffer_t* index_buffer_new(unsigned int* data, size_t count) {
    struct index_buffer_t* buf = malloc(sizeof(struct index_buffer_t));
    buf->count = count;

    glGenBuffers(1, &buf->buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->buffer_object);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return buf;
}

void index_buffer_delete(struct index_buffer_t* buf) {
    glDeleteBuffers(1, &buf->buffer_object);
    free(buf);
}

struct constant_buffer_t* constant_buffer_new(size_t sizeinBytes) {
    struct constant_buffer_t* buf = malloc(sizeof(struct constant_buffer_t));
    buf->size = sizeinBytes;

    glGenBuffers(1, &buf->buffer_object);
    glBindBuffer(GL_UNIFORM_BUFFER, buf->buffer_object);

    glBufferData(GL_UNIFORM_BUFFER, sizeinBytes, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return buf;
}

void constant_buffer_delete(struct constant_buffer_t* buf) {
    glDeleteBuffers(1, &buf->buffer_object);
    free(buf);
}

void constant_buffer_update(struct constant_buffer_t* buf, void* data) {
    glBindBuffer(GL_UNIFORM_BUFFER, buf->buffer_object);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, buf->size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

