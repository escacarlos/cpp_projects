#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "./domain_src/Map.h"
#include "./shaders/Shader.h"  // Helper class to load shaders
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Constants
const int MAP_LENGTH = 100;  // Set default map size
const int MAP_WIDTH = 100;   // Set default map size
const int GRID_SIZE = 10; // Each spot in pixels
const int WINDOW_WIDTH = MAP_LENGTH * GRID_SIZE;
const int WINDOW_HEIGHT = MAP_WIDTH * GRID_SIZE;
const float MIN_FREQUENCY = 2.4f;
const float MAX_FREQUENCY = 5.0f;
const int MIN_SIGNAL_STRENGTH = 1;
const int MAX_SIGNAL_STRENGTH = 100;

// Global Objects
wifimap::Map wifiMap(MAP_LENGTH, MAP_WIDTH);
GLuint vao, vbo;
std::vector<float> gridVertices;
GLuint shaderProgram;

// Function Prototypes
void setupGridRendering();
void generateGridData();
void renderGrid();
void processInput(GLFWwindow *window);
void checkMouseHover(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void renderUI();

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Create Window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "WiFi Map", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL with GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Set viewport
    // glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Compile and link shaders
    Shader shader("./shaders/grid_vertex.glsl", "./shaders/grid_fragment.glsl");
    shaderProgram = shader.ID;

    // Setup OpenGL buffers
    setupGridRendering();

    // Setup ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Update grid data
        generateGridData();

        // Render grid
        renderGrid();

        // Render ImGui UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        renderUI();
        checkMouseHover(window);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void setupGridRendering() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    // Vertex position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void generateGridData() {
    gridVertices.clear();
    int length = wifiMap.getLength();
    int width = wifiMap.getWidth();

    float spotSize = 2.0f * GRID_SIZE / WINDOW_WIDTH; // Normalize to OpenGL [-1,1] range
    
    for (int x = 0; x < length; x++) {
        for (int y = 0; y < width; y++) {
            // float xpos = (x / (float)length) * 2.0f - 1.0f;  
            // float ypos = (y / (float)width) * 2.0f - 1.0f;
            float left = (x * spotSize) - 1.0f;
            float right = ((x + 1) * spotSize) - 1.0f;
            float bottom = 1.0f - ((y + 1) * spotSize);
            float top = 1.0f - (y * spotSize);

            bool isAccessPoint = false;
            for (const auto& ap : wifiMap.getListAccessPoint()) {
                if (ap->getX() == x && ap->getY() == y) {
                    isAccessPoint = true;
                    break;
                }
            }
            
            glm::vec3 color;

            if (isAccessPoint) {
                // gridVertices.insert(gridVertices.end(), { xpos, ypos, 1.0f, 0.0f, 0.0f }); // Red
                color = glm::vec3(1.0f, 0.0f, 0.0f); // Red
            } else {
                // gridVertices.insert(gridVertices.end(), { xpos, ypos, 0.5f, 0.5f, 0.5f }); // Gray
                color = glm::vec3(0.5f, 0.5f, 0.5f); // Gray
            }

            // First triangle
            gridVertices.insert(gridVertices.end(), { left, bottom, color.r, color.g, color.b });
            gridVertices.insert(gridVertices.end(), { right, bottom, color.r, color.g, color.b });
            gridVertices.insert(gridVertices.end(), { right, top, color.r, color.g, color.b });

            // Second triangle
            gridVertices.insert(gridVertices.end(), { left, bottom, color.r, color.g, color.b });
            gridVertices.insert(gridVertices.end(), { right, top, color.r, color.g, color.b });
            gridVertices.insert(gridVertices.end(), { left, top, color.r, color.g, color.b });
        }
    }

    // Update OpenGL buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_DYNAMIC_DRAW);
}

void renderGrid() {
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    // glDrawArrays(GL_POINTS, 0, gridVertices.size() / 5);
    glDrawArrays(GL_TRIANGLES, 0, gridVertices.size() / 5); // 5 floats per vertex (x, y, r, g, b)
}

void renderUI() {
    static int ap_x = 0, ap_y = 0;
    static float ap_frequency = 2.4f;
    static int ap_strength = 50;

    ImGui::Begin("WiFi Map Controls");

    ImGui::InputInt("X Coordinate", &ap_x);
    ImGui::InputInt("Y Coordinate", &ap_y);
    ImGui::SliderFloat("Frequency (GHz)", &ap_frequency, MIN_FREQUENCY, MAX_FREQUENCY);
    ImGui::SliderInt("Signal Strength", &ap_strength, MIN_SIGNAL_STRENGTH, MAX_SIGNAL_STRENGTH);

    if (ImGui::Button("Create Access Point")) {
        wifiMap.addAccessPoint(ap_x, ap_y, ap_frequency, ap_strength);
    }

    ImGui::End();

    // Move the text to the top-right
    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 80, 10)); // Top-right corner
    ImGui::Begin("Access Point Count", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("AP: %d", wifiMap.getListAccessPoint().size());
    ImGui::End();

    // FPS Counter (Top-left)
    float currentTime = glfwGetTime();
    static float lastTime = 0.0f;
    static int frameCount = 0;
    frameCount++;
    int fps;

    if (currentTime - lastTime >= 1.0f) { // Update every second
        fps = frameCount;
        frameCount = 0;
        lastTime = currentTime;
    }

    ImGui::SetNextWindowPos(ImVec2(10, 10));  // Top-left corner
    ImGui::Begin("FPS Counter", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("FPS: %d", fps);
    ImGui::End();
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void checkMouseHover(GLFWwindow* window) {
    // Get mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Convert to grid coordinates
    int gridX = static_cast<int>(mouseX / GRID_SIZE);
    int gridY = static_cast<int>(mouseY / GRID_SIZE);

    // Check if an access point exists at the position
    auto accessPoint = wifiMap.getAccessPoint(gridX, gridY);
    if (accessPoint) {
        // float displayX = (gridX * GRID_SIZE) - 1.0f;
        // float displayY = 1.0f - (gridY * GRID_SIZE);

        // // Adjust position (North if possible, South otherwise)
        // float textOffsetY = (gridY > wifiMap.getWidth() / 2) ? -0.05f : 0.05f;
        float tooltipY = (gridY > wifiMap.getWidth() / 2) ? mouseY - 55 : mouseY + 15;
        float tooltipX = (gridX > wifiMap.getLength() / 2) ? mouseX - 150 : mouseX + 15;
        
        // Display tooltip
        // ImGui::SetNextWindowPos(ImVec2(displayX, displayY + textOffsetY));
        ImGui::SetNextWindowPos(ImVec2(tooltipX, tooltipY));
        ImGui::BeginTooltip();
        ImGui::Text("Coords: (%d, %d)", gridX, gridY);
        ImGui::Text("Frequency: %.1f GHz", accessPoint->getFrequency());
        ImGui::Text("Strength: %d dBm", accessPoint->getEmissionStrength());
        ImGui::End();
    }
}