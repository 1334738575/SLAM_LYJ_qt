#include "OpenGLTest.h"
#include <opencv2/opencv.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

const unsigned int SCR_WIDTH = 512;
const unsigned int SCR_HEIGHT = 512;

// ������ɫ��Դ��
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 projection;
uniform mat4 view;
void main() {
    vec4 Pc = view * vec4(aPos, 1.0);
    Pc.x = Pc.x / Pc.z;
    Pc.y = Pc.y / Pc.z;
    Pc.z = Pc.z / projection[2][3];
    gl_Position.x = (Pc.x * projection[0][0] + projection[0][2] - projection[0][3])/projection[0][3];
    gl_Position.y = (Pc.y * projection[1][1] + projection[1][2] - projection[1][3])/projection[1][3];
    gl_Position.z = Pc.z;
    gl_Position.w = 1.0f;
}
)";

// Ƭ����ɫ��Դ��,flat in uint faceID;�����ֵ
const char* fragmentShaderSource = R"(
#version 330 core
out uvec4 FragColor;
void main() {
    FragColor = uvec4(
        ((gl_PrimitiveID >> 24) & 0xFF) + 1,
        (gl_PrimitiveID >> 16) & 0xFF,
        (gl_PrimitiveID >> 8)  & 0xFF,
        gl_PrimitiveID         & 0xFF
    );
}
)";

int testGL() {
    // ��ʼ��GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW��ʼ��ʧ��" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ��������
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Core Profile Demo", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // ���ô�ֱͬ��

    // ��ʼ��GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW��ʼ��ʧ��" << std::endl;
        glfwTerminate();
        return -1;
    }

    const int indSize = 6;
    const int vSize = indSize * 3;
    float fx = 1;
    float fy = 1;
    float cx = SCR_WIDTH / 2;
    float cy = SCR_HEIGHT / 2;
    float maxD = 2.0f;
    // ���ö������ݣ�NDC���꣩
    float vertices[vSize] = {
        -100.0f, -100.f, 1.0f,  // ����
         100.f, -100.f, 1.0f,  // ����
         0.0f,  100.f, 1.0f,   // ����
         -100.0f, -100.f, 1.0f,  // ����
         100.f, -100.f, 1.0f,  // ����
         0.0f,  -200.f, 1.0f   // ����
    };

    // �������ݣ��������������Σ�
    unsigned int indices[indSize] = {
        0, 1, 2,  // ��һ��������
        3, 4, 5
    };

    // ����VAO/VBO
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

    // ������ɫ��
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

    // ����֡����
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    //������ɫ������Ƭid��
    GLuint colorTex;
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    // �����������
    GLuint depthTexture;
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    //glReadBuffer(GL_NONE);

    // ��� FBO �Ƿ�����
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // FBO ���������������
        std::cout << "error!" << std::endl;
    }

    // ��ѭ��
    while (!glfwWindowShouldClose(window)) {
        glDisable(GL_BLEND); //���8UIʱ���������
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ������ͼ�����������Z��������
        float projectM[16] = {
            fx,0,cx,SCR_WIDTH / 2,
            0,fy,cy,SCR_HEIGHT / 2,
            0,0,1,maxD,  // �����λ��z=1
            0,0,0,1
        };
        float view[16] = {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };

        //after vertex (-0.5, -0.5, 1, 1) (0.5, -0.5, 1, 1) (0, -0.5, 0.5, 1)
        //after pro (-0.5, -0.5, 1, 1) (0.5, -0.5, 1, 1) (0, -1, 1, 1)
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projectM);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, view);

        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, indSize);
        glDrawElements(GL_TRIANGLES, indSize, GL_UNSIGNED_INT, 0);

        //������ɫ
        std::vector<uchar> colors(SCR_WIDTH * SCR_HEIGHT * 4);
        glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, colors.data());
        std::vector<uint32_t> fids(SCR_WIDTH* SCR_HEIGHT, UINT_MAX);
        cv::Mat mm(SCR_HEIGHT, SCR_WIDTH, CV_8UC1);
        for (uint32_t i = 0; i < SCR_WIDTH * SCR_HEIGHT; ++i) {
            uint32_t id = i * 4;
            if (((int)colors[id + 0] + (int)colors[id + 1] + (int)colors[id + 2] + (int)colors[id + 3]) == 0)
                continue;
            uchar r = colors[id + 0];
            uchar g = colors[id + 1];
            uchar b = colors[id + 2];
            uchar a = colors[id + 3];
            int x = i % SCR_WIDTH;
            int y = i / SCR_WIDTH;
            mm.at<uchar>(y, x) = a * 100 + 100;
            uint32_t ret = ((colors[id + 0]-1) << 24) | (colors[id + 1] << 16) | (colors[id + 2] << 8) | colors[id + 3];
            fids[i] = ret;
        }

        // �������ͼ
        std::vector<float> depthData(SCR_WIDTH * SCR_HEIGHT);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());
        cv::Mat m(SCR_HEIGHT, SCR_WIDTH, CV_8UC1);
        cv::Mat m2(SCR_HEIGHT, SCR_WIDTH, CV_32FC1);
        for (int i = 0; i < SCR_WIDTH * SCR_HEIGHT; ++i) {
            //if (depthData[i] != 1.0)
                //std::cout << "111111" << std::endl;
            int x = i % SCR_WIDTH;
            int y = i / SCR_WIDTH;
            float d = (depthData[i] - 0.5f) * 2;
            m.at<uchar>(y, x) = static_cast<unsigned char>(d * 255);
            m2.at<float>(y, x) = d * maxD;
        }


        cv::imshow("fid", mm);
        cv::imshow("111", m);
        cv::waitKey();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ������Դ
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &depthTexture);
    glDeleteTextures(1, &colorTex);

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