#pragma once

#include <glad/glad.h>

class FXAA
{
    public:
        static void Setup();
        static void Begin();
        static void End();
        static void Resize(int width, int height);
        static bool IsSetup();

    private:
        static bool is_setup_;
        static GLuint texture_;
        static GLuint fbo_;
        static GLuint vao_;
        static GLuint vbo_;
        static GLuint rbo_;
};
