#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <GLES3/gl3.h>
#include "logger.h"

#define TAG "Program"

#define CHECK_GL_ERROR() LOGD(TAG, "GL Error=%x\n", glGetError())

class Program
{
    public:
        // 构造器读取并构建着色器
        static Program createByFile(const GLchar* vertexPath, const GLchar* fragmentPath) {
            // 1. 从文件路径中获取顶点/片段着色器
            std::string vertexCode;
            std::string fragmentCode;
            std::ifstream vShaderFile;
            std::ifstream fShaderFile;
            // 保证ifstream对象可以抛出异常：
            vShaderFile.exceptions(std::ifstream::badbit);
            fShaderFile.exceptions(std::ifstream::badbit);
            try {
                // 打开文件
                vShaderFile.open(vertexPath);
                fShaderFile.open(fragmentPath);
                std::stringstream vShaderStream, fShaderStream;
                // 读取文件的缓冲内容到流中
                vShaderStream << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();       
                // 关闭文件
                vShaderFile.close();
                fShaderFile.close();
                // 转换流至GLchar数组
                vertexCode = vShaderStream.str();
                fragmentCode = fShaderStream.str();     
            } catch(std::ifstream::failure e) {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
            }

            const GLchar* vShaderCode = vertexCode.c_str();
            const GLchar* fShaderCode = fragmentCode.c_str();
            return Program(vShaderCode, fShaderCode);
        }

        ~Program() {
            glDeleteProgram(mProgram);
        }

        Program(const GLchar* mVertexShaderSource, const GLchar* mFragmentShaderSource) :
            mVertexShader(0), mFragmentShader(0), mProgram(0) {
            // Vertex shader
            GLuint mVertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(mVertexShader, 1, &mVertexShaderSource, NULL);
            glCompileShader(mVertexShader);
            if (!checkShaderCompileStatus(mVertexShaderSource, mVertexShader)) {
                return ;
            }

            // Fragment shader
            GLuint mFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(mFragmentShader, 1, &mFragmentShaderSource, NULL);
            glCompileShader(mFragmentShader);
            if (!checkShaderCompileStatus(mFragmentShaderSource, mFragmentShader)) {
                return ;
            }

            mProgram = glCreateProgram();
            glAttachShader(mProgram, mVertexShader);
            glAttachShader(mProgram, mFragmentShader);
        }

        GLint link() {
            if (0 != mProgram) {
                glLinkProgram(mProgram);

                glDeleteShader(mVertexShader);
                glDeleteShader(mFragmentShader);
                mVertexShader = mFragmentShader = 0;
                //checkProgramValid(mProgram);
                return checkProgramLinkStatus(mProgram);
            }
            return GL_FALSE;
        }

        // Use program
        void use() {
            glUseProgram(mProgram);
            CHECK_GL_ERROR();
        }

        GLuint getProgram() {
            return mProgram;
        }

    private:
        GLint checkShaderCompileStatus(const char* source, GLuint shader) {
            GLint success = 0;
            GLchar infoLog[512];
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 512, NULL, infoLog);
                LOGE(TAG, "Shader:\n%s\nCompilation failed:%s\n", source, infoLog);
            }
            return success;
        }

        GLint checkProgramLinkStatus(GLuint program) {
            GLint success = 0;
            GLchar infoLog[512];
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(program, 512, NULL, infoLog);
                LOGE(TAG, "ERROR: Program linking failed: %s\n", infoLog);
            }
            return success;
        }

        GLint checkProgramValid(GLuint program) {
            GLint success = 0;
            GLchar infoLog[512];
            glValidateProgram(program);
            glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(program, 512, NULL, infoLog);
                LOGE(TAG, "ERROR: Program invalid: %s\n", infoLog);
            }
            return success;
        }

        GLuint mVertexShader;
        GLuint mFragmentShader;
        // Program ID
        GLuint mProgram;
};

#endif
