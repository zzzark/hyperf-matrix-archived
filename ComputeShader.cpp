//
// Created by zrk on 2023/11/23.
//
#define GLEW_STATIC
#include "ComputeShader.h"
#include "glUtils.h"

#include <iostream>
#include <utility>
#include <vector>
#include <sstream>
#include "tick.h"


// ------------------------------------------------------------------- //
void checkGLError(const char* file, int line) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char* errorString;
        switch (err) {
            case GL_INVALID_ENUM:                   errorString = "GL_INVALID_ENUM";                    break;
            case GL_INVALID_VALUE:                  errorString = "GL_INVALID_VALUE";                   break;
            case GL_INVALID_OPERATION:              errorString = "GL_INVALID_OPERATION";               break;
            case GL_STACK_OVERFLOW:                 errorString = "GL_STACK_OVERFLOW";                  break;
            case GL_STACK_UNDERFLOW:                errorString = "GL_STACK_UNDERFLOW";                 break;
            case GL_OUT_OF_MEMORY:                  errorString = "GL_OUT_OF_MEMORY";                   break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";   break;
            default:                                errorString = "UNKNOWN_ERROR";                      break;
        }
        printf("OpenGL error in file %s at line %d: %s\n", file, line, errorString);
    }
}
#define CHECK_GL_ERROR checkGLError(__FILE__, __LINE__)
#define CHECK_GL_ERROR_CALL(FN)     FN; \
                                    CHECK_GL_ERROR


bool checkCompile(GLuint shader)
{
    int result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);	// i: integer;  v: vector;  iv: int *
    if (result == GL_FALSE)
    {
        std::cout << " ========== COMPILE ERROR ========== " << std::endl;
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));	// memory in stack, not in pile
        glGetShaderInfoLog(shader, length, &length, message);
        std::cout << message << std::endl;
        _freea(message);
        std::cout << " =================================== " << std::endl;
        return false;
    }
    return true;
}

bool checkLink(GLuint program)
{
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (linked == GL_FALSE) {
        std::cout << " ========== LINK ERROR ========== " << std::endl;
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        // Get the error log
        if (infoLogLength > 0) {
            std::vector<char> errorMessage(infoLogLength + 1);
            glGetProgramInfoLog(program, infoLogLength, NULL, &errorMessage[0]);
            std::cout << "Program linking error: " << errorMessage.data() << std::endl;
        }
        std::cout << " ================================ " << std::endl;
        return false;
    }
    return true;
}
// ------------------------------------------------------------------- //


GLuint CreateComputeShaderProgram(const char* computeShaderSource)
{
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderSource, nullptr);
    glCompileShader(computeShader);
    if (!checkCompile(computeShader)) {
        glDeleteShader(computeShader);
        return 0;
    }

    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    if (!checkLink(computeProgram)) {
        glDeleteShader(computeShader);
        glDeleteProgram(computeProgram);
        return 0;
    }
    glDeleteShader(computeShader);
    return computeProgram;
}

GLuint CreateStorageBuffer(size_t sizeInBytes, void* data, GLenum type)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeInBytes, data, type);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    return buffer;
}

void WriteStorageBuffer(GLuint buffer, size_t sizeInBytes, void* data, size_t offset=0)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeInBytes, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ReadStorageBuffer(GLuint buffer, size_t sizeInBytes, void* data, bool useBarrier)
{
    if (useBarrier)
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeInBytes, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

class CSSourceCode {
    std::string _source;
public:
    explicit CSSourceCode(const char* source) : _source(source) {}
    explicit CSSourceCode(std::string source) : _source(std::move(source)) {}

    template<typename T>
    CSSourceCode& replace(const std::string& placeholder, const T& value) {
        std::ostringstream oss;
        oss << value;
        const std::string strValue = oss.str();

        size_t startPos = 0;
        while ((startPos = _source.find(placeholder, startPos)) != std::string::npos) {
            _source.replace(startPos, placeholder.length(), strValue);
            startPos += strValue.length();
        }
        return (*this);
    }
    explicit operator const char*() { return _source.c_str(); }
    const char* c_str() { return _source.c_str(); }
};

extern void print(const float* const M, size_t N);
extern float randf();

void cs_mm(size_t N, bool bPrint=false, size_t NTest=100) {

    const char* COMPUTE_SHADER_SOURCE = R"GLSL(
    #version 430 core

    layout(local_size_x = $LOCAL_SIZE, local_size_y = $LOCAL_SIZE) in;

    layout(std430, binding = 0) readonly buffer ABufferBlock {
        float A[];
    } ABuffer;

    layout(std430, binding = 1) readonly buffer BBufferBlock {
        float B[];
    } BBuffer;

    layout(std430, binding = 2) buffer CBufferBlock {
        float C[];
    } CBuffer;

    uniform uint n;

    void main() {
        const uint i = gl_GlobalInvocationID.y;
        const uint j = gl_GlobalInvocationID.x;
        const uint N = gl_WorkGroupSize.x * gl_NumWorkGroups.x;
        CBuffer.C[N*i + j] += ABuffer.A[N*i + n] * BBuffer.B[N*n + j];
    }
    )GLSL";

    cat::InitGLContext(cat::LOGLEVEL::HIGH);

    GLint maxStorageSize;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxStorageSize);
    std::cout << "max storage size: " << maxStorageSize << std::endl;

    const size_t LOCAL_SIZE = 8;

    CSSourceCode computeShaderSource(COMPUTE_SHADER_SOURCE);
    computeShaderSource
            .replace("$LOCAL_SIZE", LOCAL_SIZE)
            .replace("$N", N);

    auto* A = new float[N*N];
    auto* B = new float[N*N];
    auto* C = new float[N*N];
    for (size_t i = 0; i < N*N; i++) {
        A[i] = randf();
        B[i] = randf();
        C[i] = 0.0f;
    }
    GLuint ABuffer = CreateStorageBuffer(N * N * sizeof(float), A, GL_STATIC_DRAW);
    GLuint BBuffer = CreateStorageBuffer(N * N * sizeof(float), B, GL_STATIC_DRAW);
    GLuint CBuffer = CreateStorageBuffer(N * N * sizeof(float), C, GL_STREAM_DRAW);

    GLuint computeProgram = CreateComputeShaderProgram((const char*)computeShaderSource);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ABuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, BBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CBuffer);

    glUseProgram(computeProgram);
    GLint uniformLocation = glGetUniformLocation(computeProgram, "n");

    auto fn = [&]() -> void {
        for (int n = 0; n < N; n++) {
            glUniform1ui(uniformLocation, n);
            glDispatchCompute(N / LOCAL_SIZE, N / LOCAL_SIZE, 1);
            if(bPrint) glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  // this seems not necessary
        }
        ReadStorageBuffer(CBuffer, N * N * sizeof(float), C, true);
    };
    fn();

    if (!bPrint) {
        Tick tick;
        tick.Start("cs_mm");
        for (size_t nt = 0; nt < NTest; nt++) {
            fn();
        }
        tick.End(1.0/(double)NTest);
    } else {
        print(A, N); print(B, N); print(C, N);
    }

    glDeleteBuffers(1, &ABuffer);
    glDeleteBuffers(1, &BBuffer);
    glDeleteBuffers(1, &CBuffer);
    glDeleteProgram(computeProgram);

    cat::TerminateGLContext();
}
