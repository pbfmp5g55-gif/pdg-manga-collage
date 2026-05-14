#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdio>

#include <vector>

#include "app/assets.h"
#include "app/compositor.h"
#include "app/exporter.h"
#include "app/gl_loader.h"
#include "app/panel_editor.h"
#include "app/postfx.h"
#include "app/renderer.h"

namespace {

struct AppState {
    pdg::PanelEditor    editor;
    pdg::AssetLibrary   library;
    pdg::Compositor     compositor;
    pdg::SpriteRenderer renderer;
    pdg::PostFx         postfx;
    pdg::ExportPanel    exporter;
    bool showCompose = true;
    bool showPostFx  = true;
    bool showExport  = true;
    bool showEditor  = false;
};

void glfwErrorCallback(int err, const char* msg) {
    std::fprintf(stderr, "[glfw] error %d: %s\n", err, msg);
}

void dropCallback(GLFWwindow* win, int count, const char** paths) {
    auto* app = static_cast<AppState*>(glfwGetWindowUserPointer(win));
    if (app) app->editor.OnDrop(count, paths);
}

}  // namespace

int main(int, char**) {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow* win = glfwCreateWindow(
        1280, 720, "pdg-manga-collage (M4)", nullptr, nullptr);
    if (!win) { glfwTerminate(); return 1; }

    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    if (!pdgglLoad()) {
        std::fprintf(stderr, "[main] required GL entry points missing\n");
        glfwDestroyWindow(win);
        glfwTerminate();
        return 1;
    }

    AppState app;
    glfwSetWindowUserPointer(win, &app);
    glfwSetDropCallback(win, dropCallback);

    if (!app.renderer.Init() || !app.postfx.Init()) {
        std::fprintf(stderr, "[main] renderer / postfx init failed\n");
        glfwDestroyWindow(win);
        glfwTerminate();
        return 1;
    }
    app.library.Scan();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(win, GLFW_TRUE);

        int w = 0, h = 0;
        glfwGetFramebufferSize(win, &w, &h);

        // 1) Render scene into post-fx FBO.
        app.postfx.BeginCapture(w, h);
        glViewport(0, 0, w, h);
        glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        app.compositor.RegenerateIfDirty(app.library, w, h);
        app.compositor.Render(app.renderer, app.library, w, h);

        // 2) Composite to default framebuffer with effect.
        app.postfx.EndCaptureAndDraw(w, h);

        // 2.5) If user clicked Save, read the post-effect framebuffer.
        if (app.exporter.ShouldExport()) {
            std::vector<unsigned char> px((size_t)w * h * 4);
            glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
            app.exporter.SavePNG(px.data(), w, h);
            app.exporter.ConsumeExport();
        }

        // 3) ImGui on top.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Compose",      nullptr, &app.showCompose);
                ImGui::MenuItem("Post Effect",  nullptr, &app.showPostFx);
                ImGui::MenuItem("Export",       nullptr, &app.showExport);
                ImGui::MenuItem("Panel Editor", nullptr, &app.showEditor);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (app.showCompose) app.compositor.Draw(app.library);
        if (app.showPostFx)  app.postfx.Draw();
        if (app.showExport)  app.exporter.Draw();
        if (app.showEditor)  app.editor.Draw(&app.showEditor);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(win);
    }

    app.postfx.Shutdown();
    app.renderer.Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
