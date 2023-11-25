//
// Created by zrk on 2023/11/23.
//

#include "glUtils.h"
#include <iostream>
#include <windows.h>


#ifdef FORCE_USE_DEDICATED_GRAPHICS_CARD
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif


namespace cat
{
    LOGLEVEL __level = LOGLEVEL::LOW;

    void GLAPIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                    GLsizei length, const GLchar* message, const void* userParam) {
        using std::cout, std::endl;

        if (__level == LOGLEVEL::ALL) {
            // pass
        }
        else if (__level == LOGLEVEL::LOW) {
            if (severity != GL_DEBUG_SEVERITY_LOW &&
                severity != GL_DEBUG_SEVERITY_MEDIUM &&
                severity != GL_DEBUG_SEVERITY_HIGH)
            return;
        }
        else if (__level == LOGLEVEL::MEDIUM) {
            if (severity != GL_DEBUG_SEVERITY_MEDIUM &&
                severity != GL_DEBUG_SEVERITY_HIGH)
                return;
        }
        else if (__level == LOGLEVEL::HIGH) {
            if (severity != GL_DEBUG_SEVERITY_HIGH)
                return;
        }
        else if (__level == LOGLEVEL::NOLOG) {
            return;
        }

        const char* strType;
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:               strType = "ERROR";                  break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: strType = "DEPRECATED_BEHAVIOR";    break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  strType = "UNDEFINED_BEHAVIOR";     break;
            case GL_DEBUG_TYPE_PORTABILITY:         strType = "PORTABILITY";            break;
            case GL_DEBUG_TYPE_PERFORMANCE:         strType = "PERFORMANCE";            break;
            case GL_DEBUG_TYPE_OTHER:               strType = "OTHER";                  break;
            default:                                strType = "UNKNOWN TYPE";           break;
        }
        const char* strSeverity;
        switch (severity){
            case GL_DEBUG_SEVERITY_LOW:     strSeverity = "LOW";    break;
            case GL_DEBUG_SEVERITY_MEDIUM:  strSeverity = "MEDIUM"; break;
            case GL_DEBUG_SEVERITY_HIGH:    strSeverity = "HIGH";   break;
            default:                        strSeverity = "UNKNOWN SEVERITY"; break;
        }
        const int MESSAGE_MAX_NUM = 9;
        static int message_current_num = 0;

        std::cout << " ######### OPENGL ERROR " << message_current_num << " #########" << std::endl;
        std::cout << " # Debug message received:" << std::endl;
        std::cout << " # Source: " << source << std::endl;
        std::cout << " # Type: " << strType << std::endl;
        std::cout << " # ID: " << id << std::endl;
        std::cout << " # Severity: " << strSeverity << std::endl;
        std::cout << " # Message: " << message << std::endl;
        std::cout << " *Note*: This message is from file " << __FILE__ << " line " << __LINE__ << std::endl
                  << "         Toggle on a breakpoint here and check the call stack. " << std::endl;
        std::cout << " ##################################" << std::endl;

        if (++message_current_num > MESSAGE_MAX_NUM) {
            std::cout << "Too many errors, abort. " << std::endl;
            exit(1);
        }
    }

    void InitGLContext(LOGLEVEL level)
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        __level = level;
#ifdef DEBUG_OPENGL
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

        GLFWwindow* window = glfwCreateWindow(400, 300, "GLFW Hidden Window", nullptr, nullptr);
        if (window == nullptr) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(1);
        }
        glfwMakeContextCurrent(window);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            exit(1);
        }
#ifdef DEBUG_OPENGL
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(glDebugCallback, nullptr);
#endif
        std::cout << "GL_VERSION  " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GL_SHADING_LANGUAGE_VERSION  " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    }

    void TerminateGLContext()
    {
        glfwTerminate();
    }

}
