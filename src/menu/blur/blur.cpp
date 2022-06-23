#include "../imgui/imgui.h"
#include "../imgui/GL/gl3w.h"
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

static int backbufferWidth = 0;
static int backbufferHeight = 0;

[[nodiscard]] static GLuint createTexture(int width, int height) noexcept {
    GLint lastTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, lastTexture);
    return texture;
}

class ShaderProgram {
public:
    void use(float uniform, int location) const noexcept {
        glUseProgram(program);
        glUniform1f(location, uniform);
    }

    void init(const char* pixelShaderSrc, const char* vertexShaderSrc) noexcept {
        if (initialized)
            return;
        initialized = true;

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &pixelShaderSrc, nullptr);
        glCompileShader(fragmentShader);

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
        glCompileShader(vertexShader);

        program = glCreateProgram();
        glAttachShader(program, fragmentShader);
        glAttachShader(program, vertexShader);
        glLinkProgram(program);

        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
    }

private:
    GLuint program = 0;
    bool initialized = false;
};

namespace BlurEffect {
    GLint textureBackup = 0;
    GLint fboBackup = 0;
    GLint programBackup = 0;

    GLuint blurTexture1 = 0;
    GLuint blurTexture2 = 0;
    GLuint frameBuffer = 0;

    ShaderProgram blurShaderX;
    ShaderProgram blurShaderY;
    static constexpr auto blurDownsample = 4;

    void begin(const ImDrawList*, const ImDrawCmd*) noexcept {
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBackup);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fboBackup);
        glGetIntegerv(GL_CURRENT_PROGRAM, &programBackup);

        if (!frameBuffer)
            glGenFramebuffers(1, &frameBuffer);

        glViewport(0, 0, backbufferWidth / blurDownsample, backbufferHeight / blurDownsample);
        glDisable(GL_SCISSOR_TEST);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTexture1, 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, blurTexture2, 0);
        glReadBuffer(GL_BACK);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, backbufferWidth, backbufferHeight, 0, 0, backbufferWidth / blurDownsample, backbufferHeight / blurDownsample, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
    }

    void firstPass(const ImDrawList*, const ImDrawCmd*) noexcept {
        blurShaderX.use(1.0f / (backbufferWidth / blurDownsample), 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glUniform1i(1, 0);
    }

    void secondPass(const ImDrawList*, const ImDrawCmd*) noexcept {
        blurShaderY.use(1.0f / (backbufferHeight / blurDownsample), 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glUniform1i(1, 0);
    }

    void end(const ImDrawList*, const ImDrawCmd*) noexcept {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBackup);
        glUseProgram(programBackup);
        glBindTexture(GL_TEXTURE_2D, textureBackup);
    }

    void clearTextures() noexcept {
        if (blurTexture1) {
            glDeleteTextures(1, &blurTexture1);
            blurTexture1 = 0;
        }
        if (blurTexture2) {
            glDeleteTextures(1, &blurTexture2);
            blurTexture2 = 0;
        }
    }

    void createTextures() noexcept {
        if (!blurTexture1)
            blurTexture1 = createTexture(backbufferWidth / blurDownsample, backbufferHeight / blurDownsample);
        if (!blurTexture2)
            blurTexture2 = createTexture(backbufferWidth / blurDownsample, backbufferHeight / blurDownsample);
    }

    void createShaders() noexcept {
        blurShaderX.init(
        #include "blurX.glsl"
        ,
        #include "passthrough.glsl"
        );

        blurShaderY.init(
        #include "blurY.glsl"
        ,
        #include "passthrough.glsl"
        );
    }

    void draw(ImDrawList* drawList, float alpha) noexcept {
        if (const auto [width, height] = ImGui::GetIO().DisplaySize; backbufferWidth != static_cast<int>(width) || backbufferHeight != static_cast<int>(height)) {
            clearTextures();
            backbufferWidth = static_cast<int>(width);
            backbufferHeight = static_cast<int>(height);
        }

        createTextures();
        createShaders();

        if (!blurTexture1 || !blurTexture2)
            return;

        drawList->AddCallback(&begin, nullptr);
        for (int i = 0; i < 8; ++i) {
            drawList->AddCallback(&firstPass, nullptr);
            drawList->AddImage(reinterpret_cast<ImTextureID>(blurTexture1), { -1.0f, -1.0f }, { 1.0f, 1.0f });
            drawList->AddCallback(&secondPass, nullptr);
            drawList->AddImage(reinterpret_cast<ImTextureID>(blurTexture2), { -1.0f, -1.0f }, { 1.0f, 1.0f });
        }
        drawList->AddCallback(&end, nullptr);
        drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

        drawList->AddImage(reinterpret_cast<ImTextureID>(blurTexture1), { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, IM_COL32(255, 255, 255, 255 * alpha));
    }
};