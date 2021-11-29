#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>

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

      int offset_loc = glGetUniformLocation(shaderProgram, "offset");
      glm::vec2 offset;
      offset.x = glm::sin(glfwGetTime()) * 0.5;
      offset.y = glm::cos(glfwGetTime()) * 0.5;
      
      glUniform2fv(offset_loc, 1, (float *)&offset);

      int scale_loc = glGetUniformLocation(shaderProgram, "scale");
      glm::vec2 scale;
      scale.x = 1 + glm::sin(glfwGetTime()*1.13) * 0.25;
      scale.y = 1 + glm::cos(glfwGetTime()*1.71) * 0.3;

      glUniform2fv(scale_loc, 1, (float *)&scale);

      int angle_loc = glGetUniformLocation(shaderProgram, "angle");
      float angle = glfwGetTime();

      glUniform1f(angle_loc, angle);

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

uniform vec2 offset;
uniform vec2 scale;
uniform float angle;

out vec4 vsColor;
void main()
{
  vec4 p = position;
  p *= vec4(scale,1,1);
  //rotate
  vec2 new_x = vec2(cos(angle),sin(angle))*p.x;
  vec2 new_y = vec2(-sin(angle),cos(angle))*p.y;

  p.xy = new_x+new_y;

  p += vec4(offset,0,0);
  gl_Position=p;
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