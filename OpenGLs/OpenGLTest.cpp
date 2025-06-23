#include "OpenGLTest.h"
#include <opencv2/opencv.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

const unsigned int SCR_WIDTH = 512;
const unsigned int SCR_HEIGHT = 512;

// 顶点着色器源码
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

const char* vertexShaderSource2 = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
})";

// 片段着色器源码,flat in uint faceID;避免插值
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)";

const char* fragmentShaderSource2 = R"(
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

const char* fragmentShaderSource3 = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
void main() {
    FragColor = texture(texture1, TexCoord);
})";

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

    GLuint ttt = GL_UNSIGNED_BYTE;// GL_FLOAT; // GL_UNSIGNED_BYTE

    const int indSize = 6;
    const int vSize = indSize * 3;
    float fx = 1;
    float fy = 1;
    float cx = SCR_WIDTH / 2;
    float cy = SCR_HEIGHT / 2;
    float maxD = 2.0f;
    // 设置顶点数据（NDC坐标）
    float vertices[vSize] = {
        -100.0f, -100.f, 1.0f,  // 左下
         100.f, -100.f, 1.0f,  // 右下
         0.0f,  100.f, 1.0f,   // 顶部
         -100.0f, -100.f, 1.0f,  // 左下
         100.f, -100.f, 1.0f,  // 右下
         0.0f,  -200.f, 1.0f   // 顶部
    };
    // 索引数据（定义两个三角形）
    unsigned int indices[indSize] = {
        0, 1, 2,  // 第一个三角形
        3, 4, 5
    };

    // 创建VAO/VBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    //VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // 顶点位置属性，shader中的location = 0
    //EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    //unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //vertex
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    //frag
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    //frag2
    GLuint fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader2, 1, &fragmentShaderSource2, NULL);
    glCompileShader(fragmentShader2);
    //program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    GLuint shaderProgram2 = glCreateProgram();
    glAttachShader(shaderProgram2, vertexShader);
    if(ttt == GL_FLOAT)
        glAttachShader(shaderProgram2, fragmentShader);
    else
        glAttachShader(shaderProgram2, fragmentShader2);
    glLinkProgram(shaderProgram2);

    // 创建帧缓冲
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    //创建颜色纹理（面片id）
    GLuint colorTex;
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    if (ttt == GL_FLOAT)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, ttt, NULL);//四通道float/四通道uint
    else if (ttt == GL_UNSIGNED_BYTE)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA_INTEGER, ttt, NULL);//四通道uint
    else
        return 0;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    // 创建深度纹理
    GLuint depthTexture;
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    //draw attachment
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    // 检查完整性（增强错误报告）
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        const char* errorMsg = "";
        switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED: errorMsg = "Default FBO missing"; break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: errorMsg = "Attachment error"; break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: errorMsg = "No attachments"; break;
        case GL_FRAMEBUFFER_UNSUPPORTED: errorMsg = "Format combo unsupported"; break;
        }
        std::cerr << "FBO Error: " << errorMsg << " (0x" << std::hex << status << ")" << std::endl;
    }
    //unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);



    // 顶点数据（位置 + 纹理坐标）
    float vertices2[16] = {
        // 位置          // 纹理坐标
        -0.5f,  0.5f,   0.0f, 1.0f,  // 左上
         0.5f,  0.5f,   1.0f, 1.0f,  // 右上
         0.5f, -0.5f,   1.0f, 0.0f,  // 右下
        -0.5f, -0.5f,   0.0f, 0.0f   // 左下
    };
    unsigned int indices2[6] = {
        0, 1, 2,
        2, 3, 0
    };
    GLuint VAO2, VBO2, EBO2;
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    glGenBuffers(1, &EBO2);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GLuint vertexShader2 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader2, 1, &vertexShaderSource2, NULL);
    glCompileShader(vertexShader2);
    GLuint fragmentShader3 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader3, 1, &fragmentShaderSource3, NULL);
    glCompileShader(fragmentShader3);
    GLuint shaderProgram3 = glCreateProgram();
    glAttachShader(shaderProgram3, vertexShader2);
    glAttachShader(shaderProgram3, fragmentShader3);
    glLinkProgram(shaderProgram3);
    GLuint texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    cv::Mat mmm = cv::imread("D:/testLyj/QT/data/2D/images/0.png", CV_LOAD_IMAGE_UNCHANGED);
    //cv::imshow("111", mmm);
    //cv::waitKey();
    unsigned char* data = mmm.data;
    GLenum fmt = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mmm.cols, mmm.rows, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);


    // 主循环
    while (!glfwWindowShouldClose(window)) {
        glDisable(GL_BLEND); //输出8UI时，必须禁用
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        // 设置视图矩阵（摄像机在Z轴正方向）
        float projectM[16] = {
            fx,0,cx,SCR_WIDTH / 2,
            0,fy,cy,SCR_HEIGHT / 2,
            0,0,1,maxD,  // 摄像机位置z=1
            0,0,0,1
        };
        float view[16] = {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glBindVertexArray(VAO);


        //默认缓冲帧
        //after vertex (-0.5, -0.5, 1, 1) (0.5, -0.5, 1, 1) (0, -0.5, 0.5, 1)
        //after pro (-0.5, -0.5, 1, 1) (0.5, -0.5, 1, 1) (0, -1, 1, 1)
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projectM);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, view);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glReadBuffer(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glDrawArrays(GL_TRIANGLES, 0, indSize);
        glDrawElements(GL_TRIANGLES, indSize, GL_UNSIGNED_INT, 0);


        // 绑定自定义帧缓冲
        //after vertex (-0.5, -0.5, 1, 1) (0.5, -0.5, 1, 1) (0, -0.5, 0.5, 1)
        //after pro (-0.5, -0.5, 1, 1) (0.5, -0.5, 1, 1) (0, -1, 1, 1)
        glUseProgram(shaderProgram2);
        //glGetUniformLocation获取名称所在位置，1为矩阵数量，GL_FALSE列为主
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram2, "projection"), 1, GL_FALSE, projectM);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram2, "view"), 1, GL_FALSE, view);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // 替换原有glClear调用
        GLuint clearColor[4] = {0, 0, 0, 0};
        glClearBufferuiv(GL_COLOR, 0, clearColor);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, indSize, GL_UNSIGNED_INT, 0);
        if (ttt == GL_UNSIGNED_BYTE) {
            //读取颜色附件: 使用 glReadBuffer 指定颜色附件，然后通过 glReadPixels 读取。
            //读取深度附件 : 直接调用 glReadPixels 并指定 GL_DEPTH_COMPONENT 作为格式，从而读取深度数据。
            //保存颜色
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            std::vector<uchar> colors(SCR_WIDTH * SCR_HEIGHT * 4);
            glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, colors.data());
            std::vector<uint32_t> fids(SCR_WIDTH* SCR_HEIGHT, UINT_MAX);
            cv::Mat mm = cv::Mat::zeros(SCR_HEIGHT, SCR_WIDTH, CV_8UC1);
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
            cv::imshow("fid", mm);
        }
        // 保存深度图
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
        cv::imshow("111", m);
        //cv::waitKey();
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);


        glBindVertexArray(VAO2);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram3);
        glActiveTexture(GL_TEXTURE0);//激活纹理0
        if(ttt == GL_FLOAT)
            glBindTexture(GL_TEXTURE_2D, colorTex);
        else
            glBindTexture(GL_TEXTURE_2D, texture2);
        glUniform1i(glGetUniformLocation(shaderProgram3, "texture1"), 0);//绑定创建的纹理到纹理0
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


        //update
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(shaderProgram2);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(fragmentShader2);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &depthTexture);
    glDeleteTextures(1, &colorTex);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &EBO2);
    glDeleteProgram(shaderProgram3);
    glDeleteShader(vertexShader2);
    glDeleteShader(fragmentShader3);
    glDeleteTextures(1, &texture2);

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