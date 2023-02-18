#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <Windows.h>
#include <stdio.h>
#include "..\tool\CreateDump.h"

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError()) {
		std::cout << "[OpenGL Error] (" << error << ")" << function <<
			" " << file << ":" << line << std::endl;
		return false;
	}
	return true;
}

struct ShaderProgramSource
{
	std::string vertexSource;
	std::string fragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
	enum ShaderType
	{
		NONE = -1,
		VERTEX,
		FRAGMENT
	};

	std::ifstream stream(filepath);
	std::string line;
	std::stringstream ss[2];
	ShaderType type = NONE;
	while (getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos)
				type = VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = FRAGMENT;
		}
		else {
			ss[(int)type] << line << "\n";
		}
	}

	return { ss[0].str(), ss[1].str() };
}

static std::string ParseShader1(const std::string& filepath)
{
	std::ifstream stream;
	std::string line;
	std::stringstream sstream;
	while (getline(stream, line)) {
		sstream << line << "\n";
	}

	return sstream.str();
}

static unsigned int CompileShader(const std::string& source, unsigned int type)
{
	//创建OpenGL着色器
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
		std::cout << message << std::endl;
		return 0;
	}

	return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
	unsigned int fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	//为着色器程序附加着色器
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	//链接着色器程序
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main()
{
	/*char szPath[512] = { 0 };
	GetModuleFileName(NULL, szPath, sizeof(szPath) - 1);
	std::string strFilePathTmp = szPath;
	std::string strDmpPath = "";
	if (strFilePathTmp.rfind(".exe") != string::npos) {
		int nIndex = strFilePathTmp.rfind("\\");
		if (nIndex != -1) {
			strDmpPath = strFilePathTmp.substr(0, nIndex + 1);
		}
	}
	strDmpPath += "dumpfile";
	std::cout << strDmpPath << std::endl;
	CreateDump::Instance()->DeclareDumpFile(strDmpPath);*/

	GLFWwindow* window;
	if (!glfwInit()) {
		return -1;
	}

	//设置OpenGL主版本3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//设置OpenGL次版本3 
	//当前OpenGL版本3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//设置OpenGL配置为核心配置文件
	//CORE 核心配置  COMPAT 兼容性配置
	//核心配置里 0不是一个对象 兼容性配置里 0是默认对象
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	//设置调用线程里的上下文窗口刷新帧率
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK) {
		std::cout << "Error" << std::endl;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;
	
	float pos[] = {
		-0.5f, -0.5f,
		 0.5f, -0.5f,
		 0.5f,  0.5f,
		-0.5f,  0.5f
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	unsigned int VAO;
	GLCall(glGenVertexArrays(1, &VAO));
	GLCall(glBindVertexArray(VAO));

	unsigned int buffer;
	GLCall(glGenBuffers(1, &buffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW));

	GLCall(glEnableVertexAttribArray(0));
	GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (const void*)0));

	unsigned int ibo;
	GLCall(glGenBuffers(1, &ibo));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

	ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

	unsigned int shader = CreateShader(source.vertexSource, source.fragmentSource);
	GLCall(glUseProgram(shader));

	GLCall(int location = glGetUniformLocation(shader, "u_Color"));
	ASSERT(location != -1);

	GLCall(glBindVertexArray(0));
	GLCall(glUseProgram(0));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	float r = 0.0f;
	float increment = 0.05f;

	while (!glfwWindowShouldClose(window)) {
		GLCall(glClear(GL_COLOR_BUFFER_BIT));

		GLCall(glUseProgram(shader));

		GLCall(glBindVertexArray(VAO));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

		GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

		if (r > 1.0f)
			increment = -0.05f;
		else if (r < 0.0f)
			increment = 0.05f;
		r += increment;
		std::cout << r << __LINE__ << std::endl;

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteProgram(shader);

	glfwTerminate();
	return 0;
}