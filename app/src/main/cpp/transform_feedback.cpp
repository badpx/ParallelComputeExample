#include <jni.h>
#include <string>

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <unistd.h>
#include <limits>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include "Program.h"
#include "logger.h"

#define TAG "T&F"


#define CLOCKS_PER_MS   (CLOCKS_PER_SEC / 1000)
#define ITEMS_COUNT(array)  (sizeof(array) / sizeof(array[0]))
static EGLConfig eglConf;
static EGLSurface eglSurface;
static EGLContext eglCtx;
static EGLDisplay eglDisp;

#define LINEAR_FUNCTION(input, output)      output = 3.1415f * (input) + 10.24f;
#define EXPONENTIAL_FUNC(intput, output)    output = 2.0f * exp(-3.0f * (intput)) * (sin(0.4f * (intput)) + cos(-1.7f * (intput))) + 3.7f;
#define FUNCTION_EXPRESSION(func) #func

#define VERTEX_SHADER_SOURCE " \
#version 300 es \n\
precision mediump float;    \n\
layout(location = 0) in vec4 inValue; \n\
out vec4 outValue; \n\
void main() {   \n\
    //outValue = 2.0f * exp(-3.0f * inValue) * (sin(0.4f * inValue) + cos(-1.7f * inValue)) + 3.7f;   \n\
    outValue = 2.0f * inValue;   \n\
}   \n\
"

#define FRAGMENT_SHADER_SOURCE " \
#version 300 es \n\
void main() {  \n\
}  \n\
"

void compareBuffer(GLfloat* cpuBuffer, GLfloat* gpuBuffer, size_t size) {
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
    for (size_t i = 0; i < size; ++i) {
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
        LOGD(TAG, "ULP: avg: %g max: %d, min: %d\n", (double)ULPsum / size, maxULP, minULP);
    } else {
        LOGD(TAG, "WARNING: The ULP error evaluation failed due to sign mismatch!\n");
    }

    LOGD(TAG, "FLT: avg: %g max: %g, min: %g\n", errorSum / size, maxError, minError);
}

void setupEGL(int w, int h) {
    // EGL config attributes
    const EGLint confAttr[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,    // very important!
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,          // we will create a pixelbuffer surface
            EGL_RED_SIZE,   8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE,  8,
            EGL_ALPHA_SIZE, 8,     // if you need the alpha channel
            EGL_DEPTH_SIZE, 16,    // if you need the depth buffer
            EGL_NONE
    };

    // EGL context attributes
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
    };

    // surface attributes
    // the surface size is set to the input frame size
    const EGLint surfaceAttr[] = {
            EGL_WIDTH, w,
            EGL_HEIGHT, h,
            EGL_NONE
    };

    EGLint eglMajVers, eglMinVers;
    EGLint numConfigs;

    eglDisp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(eglDisp, &eglMajVers, &eglMinVers);

    LOGD(TAG, "EGL init with version %d.%d\n", eglMajVers, eglMinVers);

    // choose the first config, i.e. best config
    eglChooseConfig(eglDisp, confAttr, &eglConf, 1, &numConfigs);

    eglCtx = eglCreateContext(eglDisp, eglConf, EGL_NO_CONTEXT, ctxAttr);

    // create a pixelbuffer surface
    eglSurface = eglCreatePbufferSurface(eglDisp, eglConf, surfaceAttr);

    eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx);
    CHECK_GL_ERROR();
}

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGD(TAG, "GL %s = %s\n", name, v);
}

void printOpenGLInfo() {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);
}

int test() {
//    LOGD(TAG, "Test expression:\n%s\n", FUNCTION_EXPRESSION(EXPONENTIAL_FUNC));

//    setupEGL(480, 320);
    printOpenGLInfo();
    Program program = Program(VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE);
    const GLchar* varyings[] = {(GLchar*)"outValue"};
    glTransformFeedbackVaryings(program.getProgram(), ITEMS_COUNT(varyings), varyings, GL_INTERLEAVED_ATTRIBS);
    program.link();

    const int MAX_POWER = 17;//24;
    const int MAX_INPUT_SIZE = (const int) pow(2, MAX_POWER);
    GLfloat* data = (GLfloat*)malloc(sizeof(GLfloat) * MAX_INPUT_SIZE);
    GLfloat* cpuBuffer = (GLfloat*)malloc(sizeof(GLfloat) * MAX_INPUT_SIZE);
    GLfloat* gpuBuffer = (GLfloat*)malloc(sizeof(GLfloat) * MAX_INPUT_SIZE);

    for (int i = 0; i < MAX_INPUT_SIZE; ++i) {
        data[i] = (float)i;    // Fill input data
    }

    // Send data to GPU
    glEnableVertexAttribArray(0); // layout(location = 0)
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, data);
    // Generate output buffer in GPU
    GLuint gpuOutputBuffer = 0;
    glGenBuffers(1, &gpuOutputBuffer);
    program.use();
    glEnable(GL_RASTERIZER_DISCARD);

    LOGI(TAG, "InputPower, InputSize, TotalCpuCost, NormalizedCpuCost, GlCost, GlSyncCost, GlCopyCost, TotalGpuCost, NormalizedGpuCost\n");
    for (size_t n = 16; n <= MAX_POWER; ++n) {
        GLsizei inputSize = (GLsizei) pow(2, n);

        // ================ CPU ==================
        clock_t startTime = clock();
        for (int i = 0; i < inputSize; ++i) {
            GLfloat inValue = data[i];
            LINEAR_FUNCTION(inValue, cpuBuffer[i]);
            //EXPONENTIAL_FUNC(inValue, cpuBuffer[i]);
        }

        clock_t totalCpuTime = clock() - startTime;

        LOGD(TAG, "CPU: output[%d] = %f,\toutput[%d] = %f,\toutput[%d] = %f\n",
             1, cpuBuffer[1],
             inputSize / 2 - 1,cpuBuffer[inputSize / 2 - 1],
             inputSize - 1, cpuBuffer[inputSize - 1]);

        // ================ GPU tranform feedback ==================

        /*
        GLuint vertexBuffer = 0;
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * inputSize, data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
        */

        startTime = clock();

        glBindBuffer(GL_ARRAY_BUFFER, gpuOutputBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * inputSize, NULL, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, gpuOutputBuffer);
        glBeginTransformFeedback(GL_POINTS);

        CHECK_GL_ERROR();
        glDrawArrays(GL_POINTS, 0, inputSize);
        CHECK_GL_ERROR();
        //glFinish();
        clock_t gpCost = clock();

        glBindBuffer(GL_ARRAY_BUFFER, gpuOutputBuffer);
        float *gpuMemoryBuffer = (float*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * inputSize, GL_MAP_READ_BIT);
        clock_t syncCost = clock();
        if (NULL != gpuMemoryBuffer) {
            memcpy(gpuBuffer, gpuMemoryBuffer, sizeof(GLfloat) * inputSize);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        } else {
            LOGE(TAG, "Output buffer=%p, GL Error=%x\n", gpuMemoryBuffer, glGetError());
        }
        clock_t copyCost = clock();

        glEndTransformFeedback();

        clock_t totalGpuTime = clock() - startTime;

        LOGD(TAG, "GPU: output[%d] = %f,\toutput[%d] = %f,\toutput[%d] = %f\n", 1, gpuBuffer[1],
             inputSize / 2 - 1, gpuBuffer[inputSize / 2 - 1],
             inputSize - 1, gpuBuffer[inputSize - 1]);
        LOGI(TAG, "2^%d, %d, %ld, %f, %ld, %ld, %ld, %ld, %f\n",
                n, inputSize,
                totalCpuTime / CLOCKS_PER_MS,
                (float)totalCpuTime / totalCpuTime,
                (gpCost - startTime) / CLOCKS_PER_MS,
                (syncCost - gpCost) / CLOCKS_PER_MS,
                (copyCost - syncCost) / CLOCKS_PER_MS,
                totalGpuTime / CLOCKS_PER_MS,
                (float)totalCpuTime / totalGpuTime);
        compareBuffer(cpuBuffer, cpuBuffer, inputSize);
    }

    glDisable(GL_RASTERIZER_DISCARD);

    free(data);
    free(cpuBuffer);
    free(gpuBuffer);

    return 0;
}

GLuint gGpuOutputBuffer = 0;
GLuint gVertexBuffer = 0;
GLfloat data[] = {1, 2, 3, 4, 5, 6, 7, 8};

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

    // 使用顶点缓冲区上传输入数据
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);   // layout(location = 0) 指定索引
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    // 绑定输出缓冲到 GL_TRANSFORM_FEEDBACK_BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, gGpuOutputBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, gGpuOutputBuffer);

    // 禁止光栅化
    glEnable(GL_RASTERIZER_DISCARD);

    // 开启 Transform feedback 模式
    glBeginTransformFeedback(GL_POINTS);
    // 绘制，实际上是将顶点缓冲区中的顶点送往 Vertex Shader 计算
    glDrawArrays(GL_POINTS, 0, 8);
    // 停用 Transform feedback
    glEndTransformFeedback();

    glBindBuffer(GL_ARRAY_BUFFER, gGpuOutputBuffer);
    // 将显存里的输出缓冲区映射到内存空间
    float* gpuMemoryBuffer = (float *) glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(data), GL_MAP_READ_BIT);
    if (NULL != gpuMemoryBuffer) {
        for (int i = 0; i < 8; ++i) {
            LOGI(TAG, "Result[%d]=%f", i, gpuMemoryBuffer[i]);
        }
        // 解除内存映射
        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        LOGE(TAG, "Output buffer=%p, GL Error=%x\n", gpuMemoryBuffer, glGetError());
    }
    // 重新开启光栅化
    glDisable(GL_RASTERIZER_DISCARD);
}
