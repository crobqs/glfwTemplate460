#include "gl1.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>
#include <complex.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

struct texture {
    char name[64];
    GLuint loc;
    GLuint id;
    int width;
    int height;
    int channels;
    unsigned char * data;
};

GLfloat gMixRatio = 0.2f, gMixRatio_delta = 0.01f; GLuint gMixRatio_loc;

static void glfwSetWindowCenter( GLFWwindow * window ) {
    if(!window) return;
    int sx = 0, sy = 0;
    int px = 0, py = 0;
    int mx = 0, my = 0;
    int monitor_count = 0;
    int best_area = 0;
    int final_x = 0, final_y = 0;
    glfwGetWindowSize(window ,&sx, &sy);
    glfwGetWindowPos( window ,&px, &py);
    GLFWmonitor ** m = glfwGetMonitors(&monitor_count);
    if(!m) return;
    for(int j = 0; j < monitor_count; ++j) {
        glfwGetMonitorPos(m[j] ,&mx,&my);
        const GLFWvidmode * mode = glfwGetVideoMode(m[j]);
        if( !mode ) continue;
        int minX = fmax(mx, px);
        int minY = fmax(my, py);
        int maxX = fmin(mx+mode->width, px+sx);
        int maxY = fmin(my+mode->height, py+sy);
        int area = fmax(maxX - minX, 0) * fmax(maxY - minY, 0);
        if(area > best_area) {
            final_x = mx + (mode->width-sx)/2;
            final_y = my + (mode->height-sy)/2;
            best_area = area;
        }
    }
    if(best_area) glfwSetWindowPos(window, final_x, final_y);
    else {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        if(primary) {
            const GLFWvidmode* desktop = glfwGetVideoMode(primary);
            if(desktop) glfwSetWindowPos(window, (desktop->width-sx)/2, (desktop->height-sy)/2);
        }
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        GLint polygonMode[2] = {0};
        glGetIntegerv(GL_POLYGON_MODE, polygonMode);
        if (polygonMode[0] == GL_FILL && polygonMode[1] == GL_FILL) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else if (polygonMode[0] == GL_LINE && polygonMode[1] == GL_LINE) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        gMixRatio = fmin(1.0f, fmax(0.0f, gMixRatio + gMixRatio_delta));
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        gMixRatio = fmin(1.0f, fmax(0.0f, gMixRatio - gMixRatio_delta));
    }
}

static void framebuffersize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

/* CAUTION::malloced */
static char* readFromFile(const char* file) {
    FILE* fp = fopen(file, "r");
    if (!fp) {
        fprintf(stderr, "(%s, %d) ERROR::failed to read file \"%s\"\n", __func__, __LINE__, file);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long int bufferSize = ftell(fp) + 1;
    printf("(%s, %d) buffer created of size \"%ld\" for file \"%s\"\n", __func__, __LINE__, bufferSize, file);
    fseek(fp, 0, SEEK_SET);
    char* buffer = (char*)malloc(bufferSize);
    if (!buffer) {
        fprintf(stderr, "(%s, %d) ERROR::failed to allocate buffer for file \"%s\"\n", __func__, __LINE__, file);
        return NULL;
    }
    char c = 0;
    long int bufferIdx = 0;
    while ((c = fgetc(fp)) != EOF) {
        buffer[bufferIdx] = c;
        ++bufferIdx;
    }
    buffer[bufferSize - 1] = 0;
    return buffer;
}

static GLuint compile(const char* vertexShaderFile, const char* fragmentShaderFile) {
    char infoLog[1024] = {0};
    int success = 0;
    char* vertexShaderSource = readFromFile(vertexShaderFile);
    char* fragmentShaderSource = readFromFile(fragmentShaderFile);
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, (const char**)&vertexShaderSource, NULL);
    glShaderSource(fragmentShader, 1, (const char**)&fragmentShaderSource, NULL);
    
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, sizeof infoLog, NULL, infoLog);
        fprintf(stderr, "ERROR! vertex shader compilation failed (\"%s\")\n", vertexShaderFile);
    }

    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, sizeof infoLog, NULL, infoLog);
        fprintf(stderr, "ERROR! fragment shader compilation failed (\"%s\")\n", vertexShaderFile);
    }
    
    GLuint retProgam = glCreateProgram();
    glAttachShader(retProgam, vertexShader);
    glAttachShader(retProgam, fragmentShader);
    
    glLinkProgram(retProgam);
    glGetProgramiv(retProgam, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(retProgam, sizeof infoLog, NULL, infoLog);
        fprintf(stderr, "ERROR! program linking failed (\"%s\" \"%s\")\n", vertexShaderFile, fragmentShaderFile);
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    free(vertexShaderSource);
    free(fragmentShaderSource);
    printf("(%s, %d) freed buffer created to read \"%s\"\n", __func__, __LINE__, vertexShaderFile);
    printf("(%s, %d) freed buffer created to read \"%s\"\n", __func__, __LINE__, fragmentShaderFile);

    return retProgam;
}

static void loadTexture(struct texture* tex) {
    glGenTextures(1, &(tex->id));
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    tex->data = stbi_load(tex->name, &(tex->width), &(tex->height), &(tex->channels), 4);
    if (tex->data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(tex->data);
    } else {
        fprintf(stderr, "(%s, %d) Failed to load texture \"%s\"\n", __func__, __LINE__, tex->name);
    }
    printf("(%s, %d) \"%s\" texture loaded\n", __func__, __LINE__, tex->name);
}

static void run(void) {
    int
        screenWidth = 800,
        screenHeight = 600;
    GLFWwindow* window = NULL;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(screenWidth, screenHeight, "", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetWindowCenter(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffersize_callback);

    GLuint program = compile("shaders/tex.vert", "shaders/tex.frag");

    GLfloat vertices[] = {
        -1.0f, -1.0f,    1.0f, 0.3f, 0.0f,    0.0f, 0.0f, /* bootom-left  */
        -1.0f,  1.0f,    0.5f, 0.7f, 0.3f,    0.0f, 1.0f, /* top-left     */
         1.0f,  1.0f,    0.7f, 0.9f, 0.1f,    1.0f, 1.0f, /* top-right    */
         1.0f, -1.0f,    0.6f, 0.1f, 1.0f,    1.0f, 0.0f  /* bottom-right */
    };
    
    GLuint indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    GLuint VAO, VBO, EBO;
    
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    struct texture tex1 = {"sunsetClouds.jpg"};
    struct texture tex2 = {"minimalistScenery.jpg"};
    
    {
        stbi_set_flip_vertically_on_load(true);
        loadTexture(&tex1);
        loadTexture(&tex2);
        glUseProgram(program);
        tex1.loc = glGetUniformLocation(program, "Tex1");
        tex2.loc = glGetUniformLocation(program, "Tex2");
        glUniform1i(tex1.loc, 0);
        glUniform1i(tex2.loc, 1);
        gMixRatio_loc = glGetUniformLocation(program, "MixRatio");
    }

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glUseProgram(program);
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex1.id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex2.id);
        glUniform1f(gMixRatio_loc, gMixRatio);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &tex1.id);
    glDeleteTextures(1, &tex2.id);
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
}

struct gl1 const gl1 = {
    run
};