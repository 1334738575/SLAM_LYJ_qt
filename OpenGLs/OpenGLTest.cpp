#include "OpenGLTest.h"
#include <opencv2/opencv.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
//#include <stb_image_write.h>

const unsigned int SCR_WIDTH = 512;
const unsigned int SCR_HEIGHT = 512;

// 顶点着色器源码
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 projection;
uniform mat4 view;
void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

// 片段着色器源码
const char* fragmentShaderSource = R"(
#version 330 core
void main() {
}
)";

int testGL() {
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW初始化失败" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Core Profile Demo", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 启用垂直同步

    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW初始化失败" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 设置顶点数据（NDC坐标）
    float vertices[18] = {
        -0.5f, -0.5f, 0.0f,  // 左下
         0.5f, -0.5f, 0.0f,  // 右下
         0.0f,  0.5f, 0.25f,   // 顶部
         -0.5f, -0.5f, 0.0f,  // 左下
         0.5f, -0.5f, 0.0f,  // 右下
         0.0f,  -0.5f, 0.25f   // 顶部
    };

    // 索引数据（定义两个三角形）
    unsigned int indices[6] = {
        0, 1, 2,  // 第一个三角形
        3, 4, 5   // 第二个三角形
    };

    // 创建VAO/VBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 创建着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 创建帧缓冲
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 创建深度纹理
    GLuint depthTexture;
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        glEnable(GL_DEPTH_TEST);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_DEPTH_BUFFER_BIT);

        // 设置视图矩阵（摄像机在Z轴正方向）
        float near = 0.0f, far = 1.0f;
        float ortho[16] = {
            1,0,0,0,
            0,1,0,0,
            0,0,-2 / (far - near), -(far + near) / (far - near),
            0,0,0,1
        };
        float view[16] = {
            1,0,0,0,
            0,1,0,0,
            0,0,1,-1,  // 摄像机位置z=1
            0,0,0,1
        };

        //after vertex (-0.5, -0.5, 1, 1) (0.5, -0.5, 1, 1) (0, -0.5, 0.5, 1)
        //after pro (-0.5, -0.5, 1, 1) (0.5, -0.5, 1, 1) (0, -1, 1, 1)
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, ortho);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, view);

        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // 保存深度图
        std::vector<float> depthData(SCR_WIDTH * SCR_HEIGHT);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());

        cv::Mat m(SCR_HEIGHT, SCR_WIDTH, CV_8UC1);
        std::vector<unsigned char> imageData(SCR_WIDTH * SCR_HEIGHT);
        for (int i = 0; i < SCR_WIDTH * SCR_HEIGHT; ++i) {
            //if (depthData[i] != 1.0)
                //std::cout << "111111" << std::endl;
            imageData[i] = static_cast<unsigned char>((1.0 - depthData[i]) * 255);
            int x = i % SCR_WIDTH;
            int y = i / SCR_WIDTH;
            m.at<uchar>(y, x) = static_cast<unsigned char>((1.0 - depthData[i]) * 255);
        }
        //stbi_write_png("depth.png", SCR_WIDTH, SCR_HEIGHT, 1, imageData.data(), SCR_WIDTH);
        cv::imshow("111", m);
        cv::waitKey();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &depthTexture);

    glfwTerminate();
    return 0;
}


//#include <GL\glew.h>
//#include <GLFW/glfw3.h>
//
//#include <iostream>
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void processInput(GLFWwindow* window);
//
//// settings
//const unsigned int SCR_WIDTH = 800;
//const unsigned int SCR_HEIGHT = 600;
//
//const char* vertexShaderSource = "#version 330 core\n"
//"layout (location = 0) in vec3 aPos;\n"
//"layout (location = 1) in vec3 aColor;\n"
//"out vec3 ourColor;\n"
//"void main()\n"
//"{\n"
//"	gl_Position = vec4(aPos, 1.0);\n"
//"	ourColor = aColor;\n"
//"}\0";
//
//const char* fragmentShaderSource = "#version 330 core\n"
//"out vec4 FragColor;\n"
//"in vec3 ourColor;\n"
//"void main()\n"
//"{\n"
//" 	FragColor = vec4(ourColor, 1.0f);\n"
//"}\n\0";
//int testGL()
//{
//	// glfw: initialize and configure
//	// ------------------------------
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif
//
//	// glfw window creation
//	// --------------------
//	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
//	if (window == NULL)
//	{
//		std::cout << "Failed to create GLFW window" << std::endl;
//		glfwTerminate();
//		return -1;
//	}
//	glfwMakeContextCurrent(window);
//	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//	// glew: initialize library
//	// ---------------------------------------
//	if (glewInit() != GLEW_OK)
//	{
//		std::cout << "Failed to initialize GLEW" << std::endl;
//		return -1;
//	}
//	// build and compile our shader program
//	// ------------------------------------
//	// vertex shader
//	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
//	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//	glCompileShader(vertexShader);
//	// check for shader compile errors
//	int success;
//	char infoLog[512];
//	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
//	if (!success)
//	{
//		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
//		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
//			<< infoLog << std::endl;
//	}
//	// fragment shader
//	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//	glCompileShader(fragmentShader);
//	// check for shader compile errors
//	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
//	if (!success)
//	{
//		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
//		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
//			<< infoLog << std::endl;
//	}
//	// link shaders
//	unsigned int shaderProgram = glCreateProgram();
//	glAttachShader(shaderProgram, vertexShader);
//	glAttachShader(shaderProgram, fragmentShader);
//	glLinkProgram(shaderProgram);
//	// check for linking errors
//	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
//	if (!success)
//	{
//		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
//		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
//			<< infoLog << std::endl;
//	}
//	glDeleteShader(vertexShader);
//	glDeleteShader(fragmentShader);
//	// set up vertex data (and buffer(s)) and configure vertex attributes
//	// ------------------------------------------------------------------
//	float vertices[] = {
//		// positions // colors
//		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, // bottom right
//		-0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, // bottom left
//		0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f // top
//	};
//
//	unsigned int VBO, VAO;
//	glGenVertexArrays(1, &VAO);
//	glGenBuffers(1, &VBO);
//	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
//	glBindVertexArray(VAO);
//
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//	// position attribute
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//	// color attribute
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
//	glEnableVertexAttribArray(1);
//
//	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
//	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
//	// glBindVertexArray(0);
//
//	// as we only have a single shader, we could also just activate our shader once beforehand if we want to
//	glUseProgram(shaderProgram);
//
//	//fbo
//    GLuint fbo;
//    glGenFramebuffers(1, &fbo);
//    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//    GLuint depthTexture;
//    glGenTextures(1, &depthTexture);
//    glBindTexture(GL_TEXTURE_2D, depthTexture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
//    //glDrawBuffer(GL_NONE);
//    //glReadBuffer(GL_NONE);
//
//	// render loop
//	// -----------
//	while (!glfwWindowShouldClose(window))
//	{
//		// input
//		// -----
//		processInput(window);
//
//		// render
//		// ------
//		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT);
//
//		// render the triangle
//		glBindVertexArray(VAO);
//		glDrawArrays(GL_TRIANGLES, 0, 3);
//
//		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//		// -------------------------------------------------------------------------------
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//	}
//
//	// optional: de-allocate all resources once they've outlived their purpose:
//	// ------------------------------------------------------------------------
//	glDeleteVertexArrays(1, &VAO);
//	glDeleteBuffers(1, &VBO);
//	glDeleteProgram(shaderProgram);
//
//	// glfw: terminate, clearing all previously allocated GLFW resources.
//	// ------------------------------------------------------------------
//	glfwTerminate();
//	return 0;
//}
//
//// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
//// ---------------------------------------------------------------------------------------------------------
//void processInput(GLFWwindow* window)
//{
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, true);
//}
//
//// glfw: whenever the window size changed (by OS or user resize) this callback function executes
//// ---------------------------------------------------------------------------------------------
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	// make sure the viewport matches the new window dimensions; note that width and
//	// height will be significantly larger than specified on retina displays.
//	glViewport(0, 0, width, height);
//}