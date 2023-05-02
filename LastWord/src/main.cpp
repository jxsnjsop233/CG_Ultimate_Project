#include <cstdlib>
#include <iostream>
#include "scene_and_shading.h"

Options getOptions(int argc, char* argv[]) {
    Options options;
    options.windowTitle = "Last Word";
    options.windowWidth = 1280;
    options.windowHeight = 720;
    options.windowResizable = true;
    options.vSync = true;
    options.msaa = true;
    options.glVersion = {3, 3};
    options.backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    // options.assetRootDir = "../media/";

    return options;
}

int main(int argc, char* argv[]) {
    Options options = getOptions(argc, argv);
    glfwSwapInterval(1);

    try {
        scene_and_shading app(options);
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