#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <glm/gtx/transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::printf;
using namespace glm;

GLFWwindow *window = nullptr;
GLuint shaderProgram = 0;
GLuint VAO;
GLuint VBO;
GLuint texture;

struct VertexAttrib
{
  vec3 pos;
  vec2 uv;
};

bool use_my_mat = true;

GLuint LoadShader(GLenum shaderType, const char *shaderSrc);
GLuint CreateShaderProgram();
void InitializeResource();

void OnKey(GLFWwindow *, int key, int scancode, int action, int mod)
{
  if (GLFW_PRESS == action && key == GLFW_KEY_T)
  {
    use_my_mat = !use_my_mat;
    printf("use my matrix: %d\n", use_my_mat ? 1 : 0);
  }
}

int main(int argc, const char **argv)
{
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_SAMPLES, 4);

  window = glfwCreateWindow(800, 600, "opengl", nullptr, nullptr);

  if (!window)
  {
    printf("glfw create window failed!\n");
    return -1;
  }

  glfwSetKeyCallback(window, OnKey);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
  {
    printf("glew init faield!\n");
    return -1;
  }

  InitializeResource();

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    int width = 0;
    int height = 0;
    glfwGetWindowSize(window, &width, &height);
    if (width && height)
    {
      glViewport(0, 0, width, height);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(shaderProgram);

      glm::vec2 offset;
      offset.x = glm::sin(glfwGetTime()) * 0.5;
      offset.y = glm::cos(glfwGetTime()) * 0.5;

      glm::vec2 scale;
      scale.x = 1 + glm::sin(glfwGetTime() * 1.13) * 0.25;
      scale.y = 1 + glm::cos(glfwGetTime() * 1.71) * 0.3;

      float angle = glfwGetTime();

      glm::mat4 model(1);

      model = glm::translate(glm::mat4(1), glm::vec3(offset, 0)) * glm::rotate(glm::mat4(1), angle, glm::vec3(0, 0, 1)) * glm::scale(glm::mat4(1), glm::vec3(scale, 1));

      model = glm::mat4(1);
      int model_loc = glGetUniformLocation(shaderProgram, "model");

      glUniformMatrix4fv(model_loc, 1, GL_FALSE, (float *)&model);

      glm::mat4 view(1);

      glm::vec3 center = glm::vec3(0, 0, 0);
      glm::vec3 pos = glm::normalize(glm::vec3(glm::sin(glfwGetTime()), 0.5, glm::cos(glfwGetTime()))) * 5.0f;
      glm::vec3 up = glm::vec3(0, 1, 0);

      glm::vec3 forward = glm::normalize(pos - center);
      glm::vec3 side = glm::normalize(glm::cross(up, forward));
      up = glm::normalize(glm::cross(forward, side));

      view = glm::mat4(glm::transpose(glm::mat3(side, up, forward))) * glm::translate(glm::mat4(1), -pos);

      int view_loc = glGetUniformLocation(shaderProgram, "view");
      // view = glm::mat4(1);
      glUniformMatrix4fv(view_loc, 1, GL_FALSE, (float *)&view);

      glm::mat4 projection(1);

      float near = -4.0, far = -6.0;
      projection = glm::scale(glm::mat4(1), glm::vec3(float(height) / width, 1, 1)) *
                   glm::scale(glm::mat4(1), glm::vec3(1, 1, 2 / (near - far))) *
                   glm::translate(glm::mat4(1), glm::vec3(0, 0, -(near + far) * 0.5));

      glm::mat4 p(1);
      p[2][3] = (near - far) / (2 * near);
      p[3][3] = (near + far) / (2 * near);

      p = glm::scale(glm::mat4(1), glm::vec3(1, 1, 2 / (1 + near / far))) * glm::translate(glm::mat4(1), glm::vec3(0, 0, -(1 - near / far) * 0.5)) * p;

      projection = p * projection;

      //projection[0][2] *= -1;
      //projection[1][2] *= -1;
      //projection[2][2] *= -1;
      //projection[3][2] *= -1;

      int projection_loc = glGetUniformLocation(shaderProgram, "projection");

      glUniformMatrix4fv(projection_loc, 1, GL_FALSE, (float *)&projection);

      glm::mat4 my_VP = projection * view;
      glm::mat4 glm_V = glm::lookAt(pos, center, up);
      glm::mat4 glm_P = glm::frustum(-float(width) / height, float(width) / height, -1.0f, 1.0f, -near, -far);
      glm::mat4 glm_VP = glm_P * glm_V;

      if (!use_my_mat)
      {
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, (float *)&glm_V);
        glUniformMatrix4fv(projection_loc, 1, GL_FALSE, (float *)&glm_P);
      }

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D,texture);
      int tex_loc = glGetUniformLocation(shaderProgram, "tex");
      glUniform1i(tex_loc, 0);

      glBindVertexArray(VAO);

      glDrawArrays(GL_TRIANGLES, 0, 9);
    }
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}

void InitializeResource()
{
  shaderProgram = CreateShaderProgram();

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

 /*  // triangle
  VertexAttrib datas[] = {
      {{0, 0.5, 0}, {1, 0, 0}},
      {{-0.5, -0.5, 0}, {0, 1, 0}},
      {{0.5, -0.5, 0}, {0, 0, 1}},
  };
*/

  // corner
  VertexAttrib datas[] = {
      {{0, 0, 0}, {1, 1}},
      {{0, 0.5, 0}, {1, 0}},
      {{0, 0, 0.5}, {0,1}},
      {{0, 0, 0}, {1, 1}},
      {{0.5, 0, 0}, {1, 0}},
      {{0, 0.5, 0}, {0, 1}},
      {{0, 0, 0}, {1, 1}},
      {{0.5, 0, 0}, {1, 0}},
      {{0, 0, 0.5}, {0, 1}},
  };

  glBufferData(GL_ARRAY_BUFFER, sizeof(datas), datas, GL_STATIC_DRAW);

  int posLoc = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttrib), 0);
  int uvLoc = glGetAttribLocation(shaderProgram, "uv");
  glEnableVertexAttribArray(uvLoc);
  glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttrib), (void *)(sizeof(vec3)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  {
    int w=0,h=0;
    unsigned char *pixels = stbi_load("box.jpg",&w,&h,0,3);
    if(pixels)
    {
      glGenTextures(1,&texture);
      glBindTexture(GL_TEXTURE_2D,texture);
      glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,w,h,0,GL_RGB,GL_UNSIGNED_BYTE,pixels);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);  
      glGenerateMipmap(GL_TEXTURE_2D);
    }
  }

  glClearColor(0.2, 0.3, 0.4, 1);
  glClearDepth(0.0f);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GEQUAL);
}

GLuint LoadShader(GLenum shaderType, const char *shaderSrc)
{
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &shaderSrc, 0);
  glCompileShader(shader);
  int logLen = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
  if (logLen)
  {
    std::vector<char> s(logLen);
    glGetShaderInfoLog(shader, logLen, 0, s.data());
    printf("compile shader error:\n%s\n", s.data());
    return 0;
  }
  return shader;
}

GLuint CreateShaderProgram()
{
  GLuint program = glCreateProgram();

  const char *vertSrc = R"(
#version 460 core
in vec4 position;
in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 vsUv;
void main()
{
  gl_Position=projection * view * model * position;
  vsUv=uv;
}
  )";
  const char *fragSrc = R"(
#version 460 core
in vec2 vsUv;
out vec4 fragColor;

uniform sampler2D tex;

void main()
{
  fragColor=texture(tex,vsUv);
}
  )";

  GLuint vertShader = LoadShader(GL_VERTEX_SHADER, vertSrc);
  GLuint fragShader = LoadShader(GL_FRAGMENT_SHADER, fragSrc);
  if (vertShader && fragShader)
  {
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);
    int logLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
    if (logLen)
    {
      std::vector<char> s(logLen);
      glGetProgramInfoLog(program, logLen, 0, s.data());
      printf("link program error:\n%s\n", s.data());
      return 0;
    }
    else
    {
      return program;
    }
  }

  return 0;
}