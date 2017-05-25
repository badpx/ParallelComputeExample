#include <jni.h>
#include <string>

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <unistd.h>
#include <limits>
#include <GLES3/gl3.h>
#include "Program.h"
#include "logger.h"
#include "Timer.h"

#define TAG "T&F"

#define FREE(ptr)  free(ptr); (ptr) = NULL
#define ITEMS_COUNT(array)  (sizeof(array) / sizeof(array[0]))

// Vertex shader for computing
#define VERTEX_SHADER_SOURCE " \
#version 300 es \n\
precision mediump float;    \n\
layout(location = 0) in vec4 inValue; \n\
out vec4 outValue; \n\
void main() {   \n\
//    outValue = 2.0f * exp(-3.0f * inValue) * (sin(0.4f * inValue) + cos(-1.7f * inValue)) + 3.7f;   \n\
    outValue = inValue * 3.14159f + 0.5f;   \n\
    //float gray = inValue.r * 0.3 + inValue.g * 0.59 + inValue.b * 0.11 + inValue.a * 1.0;    \n\
    //outValue = vec4(gray, gray, gray, gray);   \n\
}   \n\
"

// Empty fragment shader
#define FRAGMENT_SHADER_SOURCE " \
#version 300 es \n\
void main() {  \n\
}  \n\
"

void compareBuffer(GLfloat* cpuBuffer, GLfloat* gpuBuffer, size_t itemNum) {
    union Float_t {
        GLfloat f;
        int32_t i;
    };

    bool isCalculationCorrect = true;

    float errorSum = 0.0f;
    float maxError = 0.0f;
    float minError = std::numeric_limits<float>::max();

    int32_t ULPsum = 0;
    int32_t maxULP = 0;
    int32_t minULP = std::numeric_limits<int32_t>::max();

    union Float_t cpu, gpu;
    for (size_t i = 0; i < itemNum; ++i) {
        cpu.f = cpuBuffer[i];
        gpu.f = gpuBuffer[i];

        float error = fabsf((cpu.f - gpu.f) / cpu.f);

        maxError = std::max(maxError, error);
        minError = std::min(minError, error);

        errorSum += error;

        if ((cpu.i >> 31 != 0) != (gpu.i >> 31 != 0)) {
            if (cpu.f != gpu.f) {
                isCalculationCorrect = false;   // different sign, can't compare
            }
        }

        int32_t ulpError = abs(cpu.i - gpu.i);
        maxULP = std::max(maxULP, ulpError);
        minULP = std::min(minULP, ulpError);

        ULPsum += ulpError;
    }

    if (isCalculationCorrect) {
        LOGD(TAG, "ULP: avg: %g max: %d, min: %d\n", (double)ULPsum / itemNum, maxULP, minULP);
    } else {
        LOGD(TAG, "WARNING: The ULP error evaluation failed due to sign mismatch!\n");
    }

    LOGD(TAG, "FLT: avg: %g max: %g, min: %g\n", errorSum / itemNum, maxError, minError);
}

#define PRINT_SAMPLING(buffer, size) \
    LOGD(TAG, #buffer":[%ld] = %f, [%ld] = %f, [%ld] = %f, [%ld] = %f, [%ld] = %f", \
         0L, (buffer)[0],    \
         (size) / 2 - (size) / 4, (buffer)[(size) / 2 - (size) / 4],   \
         (size) / 2, (buffer)[(size) / 2],   \
         (size) / 2 + (size) / 4, (buffer)[(size) / 2 + (size) / 4],   \
         (size) - 1, (buffer)[(size) - 1])

void dataVerify(float *bufferA, float *bufferB, size_t size) {
    if (NULL != bufferA && NULL != bufferB) {
        PRINT_SAMPLING(bufferA, size);
        PRINT_SAMPLING(bufferB, size);

        compareBuffer(bufferA, bufferB, size);
    }
}

GLuint gGpuOutputBuffer = 0;
GLuint gVertexBuffer = 0;

extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_parallelcomputedemo_TransformFeedback_MyGLRenderer_setup(
        JNIEnv *env,
        jobject /* this */) {

    glGenBuffers(1, &gVertexBuffer);
    glGenBuffers(1, &gGpuOutputBuffer);

    Program program = Program(VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE);
    const GLchar* varyings[] = {(GLchar*)"outValue"};
    // 确定 Transform Feedback 要获取的 Shader 变量名及属性
    glTransformFeedbackVaryings(program.getProgram(), ITEMS_COUNT(varyings), varyings, GL_INTERLEAVED_ATTRIBS);
    program.link();
    program.use();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tencent_parallelcomputedemo_TransformFeedback_MyGLRenderer_computing(
        JNIEnv *env,
        jobject /* this */) {
    const size_t POW_MIN = 17;
    const size_t POW_MAX = 25;
    GLfloat* inputData = NULL;
    float* gpuOutputData = NULL;
    float* cpuOutputData = NULL;
    Timer timer;

    LOGI(TAG, "pow, upload, computing, download, GpuTotal, CpuTotal, GpuNormalized, CpuNormalized");
    for (int n = POW_MIN; n <= POW_MAX; ++n) {
        const size_t inputElementNum = (const size_t) (const int) pow(2, n);
        const size_t inputDataSize = sizeof(GLfloat) * inputElementNum;

        // 分配输入数据缓冲
        inputData = (GLfloat *) malloc(inputDataSize);
        if (NULL == inputData) {
            LOGE(TAG, "Alloc input buffer size = %ld out of memory!", inputDataSize);
            break;
        }
        // 填充输入数据
        for (int j = 0; j < inputElementNum; ++j) {
            inputData[j] = j;
        }

        if (NULL == (cpuOutputData = (float*)malloc(inputDataSize))) {
            LOGE(TAG, "Alloc CPU output buffer size = %ld out of memory!", inputDataSize);
            break;
        }

        // ====================================== CPU 计算 ==========================================
        timer.start();
#if 1
        for (int i = 0; i < inputElementNum; ++i) {
            float inValue = inputData[i];
            cpuOutputData[i] =
//                    (float) (2.0f * exp(-3.0f * inValue) * (sin(0.4f * inValue) + cos(-1.7f * inValue)) + 3.7f);
                    inValue * 3.14159f + 0.5f;
        }
#else
        for (int i = 0; i < inputElementNum; i += 4) {
            float* inValue = inputData + i;
            float gray = inValue[0] * 0.3f + inValue[1] * 0.59f + inValue[2] * 0.11f + inValue[3] * 1.0f;
            float* outValue = cpuOutputData + i;
            outValue[0] = outValue[1] = outValue[2] = outValue[3] = gray;
        }
#endif
        long cpuCost = timer.reset();

        // ====================================== GPU 计算 ==========================================
        // 使用顶点缓冲区上传输入数据
        glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, inputDataSize, inputData, GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);   // layout(location = 0) 指定索引
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
        long inputUploadCost = timer.deltaMetering();

        // 绑定输出缓冲到 GL_TRANSFORM_FEEDBACK_BUFFER
        glBindBuffer(GL_ARRAY_BUFFER, gGpuOutputBuffer);
        glBufferData(GL_ARRAY_BUFFER, inputDataSize, NULL, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, gGpuOutputBuffer);

        // 禁止光栅化
        glEnable(GL_RASTERIZER_DISCARD);

        // 开启 Transform feedback 模式
        glBeginTransformFeedback(GL_POINTS);
        // 绘制，实际上是将顶点缓冲区中的顶点送往 Vertex Shader 计算
        glDrawArrays(GL_POINTS, 0, inputElementNum);
        // 停用 Transform feedback
        glEndTransformFeedback();
        glFinish();
        long computingCost = timer.deltaMetering();

        glBindBuffer(GL_ARRAY_BUFFER, gGpuOutputBuffer);
        // 将显存里的输出缓冲区映射到内存空间
        float* gpuMemoryBuffer = (float *) glMapBufferRange(GL_ARRAY_BUFFER, 0, inputDataSize, GL_MAP_READ_BIT);
        if (NULL != gpuMemoryBuffer) {
            gpuOutputData = (float*)malloc(inputDataSize);
            if (NULL != gpuOutputData) {
                memcpy(gpuOutputData, gpuMemoryBuffer, inputDataSize);
                long memCopyCost = timer.deltaMetering();
                long gpuCost = timer.duration();
//                LOGI(TAG, "pow, upload, computing, download, GpuTotal, CpuTotal, GpuNormalized, CpuNormalized");
                LOGI(TAG, "2^%d, %ld, %ld, %ld, %ld, %ld, %.2f, 1.0",
                     n, inputUploadCost, computingCost, memCopyCost, gpuCost, cpuCost, (float)gpuCost / cpuCost);
            } else {
                LOGD(TAG, "Alloc GPU output buffer(size=%ld) out of memory!", inputDataSize);
                break;
            }
            // 解除内存映射
            glUnmapBuffer(GL_ARRAY_BUFFER);
        } else {
            LOGE(TAG, "Output buffer=%p, GL Error=%x\n", gpuMemoryBuffer, glGetError());
        }

        // 重新开启光栅化
        glDisable(GL_RASTERIZER_DISCARD);

        dataVerify(cpuOutputData, gpuOutputData, inputElementNum);
        FREE(inputData);
        FREE(gpuOutputData);
        FREE(cpuOutputData);
    }

    FREE(inputData);
    FREE(gpuOutputData);
    FREE(cpuOutputData);

}
