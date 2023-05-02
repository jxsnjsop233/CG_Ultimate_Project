#include "hello_triangle.h"
#include <iostream>

Options getOptions(int argc, char* argv[]) {
    Options options;
    options.windowTitle = "Last Word";
    options.windowWidth = 1280;
    options.windowHeight = 720;
    options.windowResizable = true;
    options.vSync = true;
    options.glVersion = {3, 3};
    options.backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    return options;
}

int main(int argc, char* argv[]) {
    glfwSwapInterval(1);
    Options options = getOptions(argc, argv);

    try {
        HelloTriangle app(options);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}