#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include <glm/gtx/transform.hpp>

using std::printf;
using namespace glm;

GLFWwindow *window = nullptr;
GLuint shaderProgram = 0;
GLuint VAO;
GLuint VBO;

struct VertexAttrib
{
  vec3 pos;
  vec3 color;
};

GLuint LoadShader(GLenum shaderType, const char *shaderSrc);
GLuint CreateShaderProgram();
void InitializeResource();

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
      glm::vec3 pos = glm::vec3(glm::sin(glfwGetTime()), 0, glm::cos(glfwGetTime()))*0.5f;
      glm::vec3 up = glm::vec3(0, 1, 0);

      glm::vec3 forword = glm::normalize(center - pos);
      glm::vec3 side = glm::normalize(glm::cross(forword, up));
      up = glm::normalize(glm::cross(side, forword));

      view = glm::mat4(glm::transpose(glm::mat3(side, up, forword))) * glm::translate(glm::mat4(1), -pos);

      int view_loc = glGetUniformLocation(shaderProgram, "view");
      //view = glm::mat4(1);
      glUniformMatrix4fv(view_loc, 1, GL_FALSE, (float *)&view);

      glBindVertexArray(VAO);

      glDrawArrays(GL_TRIANGLES, 0, 3);
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

  VertexAttrib datas[] = {
      {{0, 0.5, 0}, {1, 0, 0}},
      {{-0.5, -0.5, 0}, {0, 1, 0}},
      {{0.5, -0.5, 0}, {0, 0, 1}},
  };

  glBufferData(GL_ARRAY_BUFFER, sizeof(datas), datas, GL_STATIC_DRAW);

  int posLoc = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttrib), 0);
  int colorLoc = glGetAttribLocation(shaderProgram, "color");
  glEnableVertexAttribArray(colorLoc);
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttrib), (void *)(sizeof(vec3)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glClearColor(0.2, 0.3, 0.4, 1);
  glClearDepth(1.0f);
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
in vec4 color;

uniform mat4 model;
uniform mat4 view;

out vec4 vsColor;
void main()
{
  gl_Position=view * model * position;
  vsColor=color;
}
  )";
  const char *fragSrc = R"(
#version 460 core
in vec4 vsColor;
out vec4 fragColor;
void main()
{
  fragColor=vsColor;
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