//#include "include/GL/glew.h"
//#include "include/GLFW/glfw3.h"
#include "OpenGLInit.h"
#include "ReadSource.h"
#include "Mesh.h"
#include "Shader.h"
#include "Program.h"
#include "GLM/glm.hpp"
#include "Model.h"
#include "Cam.h"
#include <GLM/gtc/type_ptr.hpp>
int main()
{
    GLFWwindow* window = nullptr;
    if (OpenGLInit::Init(window) == false)
        return -1;

    glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    float vertices[] = {
        //  x      y     z     nx    ny    nz
         0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
    };

    unsigned int indices[] = {  // 0부터 시작한다는 것을 명심하세요!
    0, 1, 3,   // 첫 번째 삼각형
    1, 2, 3    // 두 번째 삼각형
    };

    Model model("./resource/mandibular2.stl");

    //Mesh obj(vertices, sizeof(vertices), indices, sizeof(indices));
    Mesh obj(model.GetVertices().data(), model.GetVertices().size() * sizeof(float), model.GetIndices().data(), model.GetIndices().size() * sizeof(unsigned int));
    Shader vertexShader("./VertexShader.glsl", GL_VERTEX_SHADER);
    Shader fragmentShader("./FragmentShader.glsl", GL_FRAGMENT_SHADER);
    Program program;

    program.AttachShader(vertexShader.GetShader());
    program.AttachShader(fragmentShader.GetShader());
    program.Link();

    glDeleteShader(vertexShader.GetShader());
    glDeleteShader(fragmentShader.GetShader());

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    Cam cam(model.GetCenter() + glm::vec3(0, 0, -1) * 2.f * model.GetDist(), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), 0.01f, 1000, glm::radians(60.f), aspect);


    glfwFocusWindow(window);
    int vertexColorLocation = glGetUniformLocation(program.GetProgram(), "vertexColor");
    int viewLocation = glGetUniformLocation(program.GetProgram(), "view");
    int projLocation = glGetUniformLocation(program.GetProgram(), "projection");
    glEnable(GL_DEPTH_TEST);

    glFrontFace(GL_CW);
	while (!glfwWindowShouldClose(window))
	{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program.GetProgram());
        float timeValue = glfwGetTime();
        glUniform4f(vertexColorLocation, 1, 1, 1, 1);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(cam.GetViewMat()));
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(cam.GetProjMat()));

        glBindVertexArray(obj.GetVAO());
        glDrawElements(GL_TRIANGLES, model.GetIndices().size(), GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

