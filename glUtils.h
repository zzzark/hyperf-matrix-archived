//
// Created by zrk on 2023/11/23.
//

#ifndef HYPERF_MATRIX_GLUTILS_H
#define HYPERF_MATRIX_GLUTILS_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


namespace cat {
    enum class LOGLEVEL {
        ALL,
        LOW,
        MEDIUM,
        HIGH,
        NOLOG,
    };

    extern void InitGLContext(LOGLEVEL level=LOGLEVEL::LOW);

    extern void TerminateGLContext();
}


#endif //HYPERF_MATRIX_GLUTILS_H
