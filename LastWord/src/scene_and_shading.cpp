#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <unordered_map>

#include "base/vertex.h"
#include "tiny_obj_loader.h"
#include "scene_and_shading.h"

const std::string modelRelPath = "../media/AA12_Shotgun.obj";

scene_and_shading::scene_and_shading(const Options& options) : Application(options) {
    
    // // use tiny_obj_loader to load mesh data
    // tinyobj::attrib_t attrib;
    // std::vector<tinyobj::shape_t> shapes;
    // std::vector<tinyobj::material_t> materials;

    // std::string warn, err;

    // std::string modelPath = getAssetFullPath(modelRelPath);
    // std::string::size_type index = modelPath.find_last_of("/");
    // std::string mtlBaseDir = modelPath.substr(0, index + 1);

    // if (!tinyobj::LoadObj(
    //         &attrib, &shapes, &materials, &warn, &err, modelPath.c_str(), mtlBaseDir.c_str())) {
    //     throw std::runtime_error("load " + modelPath + " failure: " + err);
    // }

    // if (!warn.empty()) {
    //     std::cerr << "Loading model " + modelPath + " warnings: " << std::endl;
    //     std::cerr << warn << std::endl;
    // }

    // if (!err.empty()) {
    //     throw std::runtime_error("Loading model " + modelPath + " error:\n" + err);
    // }



    // std::vector<Vertex> vertices;
    // std::vector<uint32_t> indices;
    // std::unordered_map<Vertex, uint32_t> uniqueVertices;

    // for (const auto& shape : shapes) {
    //     for (const auto& index : shape.mesh.indices) {
    //         Vertex vertex{};

    //         vertex.position.x = attrib.vertices[3 * index.vertex_index + 0];
    //         vertex.position.y = attrib.vertices[3 * index.vertex_index + 1];
    //         vertex.position.z = attrib.vertices[3 * index.vertex_index + 2];

    //         if (index.normal_index >= 0) {
    //             vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
    //             vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
    //             vertex.normal.z = attrib.normals[3 * index.normal_index + 2];
    //         }

    //         if (index.texcoord_index >= 0) {
    //             vertex.texCoord.x = attrib.texcoords[2 * index.texcoord_index + 0];
    //             vertex.texCoord.y = attrib.texcoords[2 * index.texcoord_index + 1];
    //         }

    //         // check if the vertex appeared before to reduce redundant data
    //         if (uniqueVertices.count(vertex) == 0) {
    //             uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
    //             vertices.push_back(vertex);
    //         }

    //         indices.push_back(uniqueVertices[vertex]);
    //     }
    // }

// ---------------------------------------------------------------------------------

    // set input mode
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // init cameras
    _cameras.resize(2);

    const float aspect = 1.0f * _windowWidth / _windowHeight;
    constexpr float znear = 0.1f;
    constexpr float zfar = 10000.0f;

    // perspective camera
    _cameras[0].reset(new PerspectiveCamera(glm::radians(60.0f), aspect, 0.1f, 10000.0f));

    _cameras[0]->transform.position = glm::vec3(0.0f, 0.0f, 15.0f);

    // orthographic camera
    _cameras[1].reset(
        new OrthographicCamera(-4.0f * aspect, 4.0f * aspect, -4.0f, 4.0f, znear, zfar));
    _cameras[1]->transform.position = glm::vec3(0.0f, 0.0f, 15.0f);

    // init model
    _rifle.reset(new Model(getAssetFullPath(modelRelPath)));

    // init shader
    initShader();
}

void scene_and_shading::handleInput() {
    constexpr float cameraMoveSpeed = 5.0f;
    constexpr float cameraRotateSpeed = 0.02f;
    constexpr float sensitivity = 0.02f;
    static float runSpeed = 1.0f;
    
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    // 缩放功能（暂时废弃）
    // if (_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS) {
    //     std::cout << "switch camera" << std::endl;
    //     // switch camera
    //     activeCameraIndex = (activeCameraIndex + 1) % _cameras.size();
    //     _input.keyboard.keyStates[GLFW_KEY_SPACE] = GLFW_RELEASE;
    //     return;
    // }

    static Camera* camera = _cameras[activeCameraIndex].get();
    static glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    static float yaw = 0.0f;
    static float pitch = 0.0f;
    // 取得任意时刻相机的前、右、上方向的单位向量
    glm::vec3 forwardDirection = glm::normalize(camera->transform.getFront());
    glm::vec3 rightDirection = glm::normalize(camera->transform.getRight());
    glm::vec3 upDirection = glm::normalize(camera->transform.getUp());
    // ------------------------------------------------------
    // 这里用于计算相机右向向量，Transform库里的getRight()函数算不出来预期的答案，不知道为啥
    glm::vec3 cameraRight = glm::normalize(glm::cross(forwardDirection, up)); 
    // ------------------------------------------------------
    

    // 转视角
    if (_input.mouse.move.xNow != _input.mouse.move.xOld) {
        float xRot = _input.mouse.move.xNow - _input.mouse.move.xOld;
        xRot *= cameraRotateSpeed;
        yaw += xRot;
    } if (_input.mouse.move.yNow != _input.mouse.move.yOld) {
        float yRot = _input.mouse.move.yNow - _input.mouse.move.yOld;
        yRot *= cameraRotateSpeed;
        pitch += yRot;
    } if(pitch > 89.0f) {
        pitch =  89.0f;
    } if(pitch < -89.0f) {
        pitch = -89.0f;
    }
    // 利用偏航角和俯仰角确定targrt的坐标
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    camera->transform.lookAt(camera->transform.position-glm::normalize(newFront), up);
    std::cout<<newFront.x<<" "<<newFront.y<<" "<<newFront.z<<std::endl;

    // 将WASD操作限定在xOz平面内
    glm::vec3 plainForwardDirection = glm::normalize(glm::vec3(forwardDirection.x, 0.0f, forwardDirection.z));
    glm::vec3 plainRightDirection = glm::normalize(glm::vec3(rightDirection.x, 0.0f, rightDirection.z));

    // 按住左shift疾跑（限定只有前后两个方向可疾跑）
    if (_input.keyboard.keyStates[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS) {
        runSpeed = 2.0f;
    } if (_input.keyboard.keyStates[GLFW_KEY_LEFT_SHIFT] == GLFW_RELEASE) {
        runSpeed = 1.0f;
    }

    // 当前相机视角前进
    if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
        camera->transform.position += cameraMoveSpeed * runSpeed * sensitivity * plainForwardDirection;
    }

    // 当前相机视角右移
    if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
        camera->transform.position -= cameraMoveSpeed * sensitivity * plainRightDirection;
    }

    // 当前相机视角后退
    if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
        camera->transform.position -= cameraMoveSpeed * runSpeed * sensitivity * plainForwardDirection;
    }

    // 当前相机视角左移
    if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
        camera->transform.position += cameraMoveSpeed * sensitivity * plainRightDirection;
    }

    // 当前相机视角上升
    if (_input.keyboard.keyStates[GLFW_KEY_SPACE] != GLFW_RELEASE) {
        camera->transform.position += cameraMoveSpeed * sensitivity * up;
    }

    // 当前相机视角左移
    if (_input.keyboard.keyStates[GLFW_KEY_LEFT_CONTROL] != GLFW_RELEASE) {
        camera->transform.position -= cameraMoveSpeed * sensitivity * up;
    }


    _input.forwardState();
}
void scene_and_shading::renderFrame() {
    showFpsInWindowTitle();

    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = _cameras[activeCameraIndex]->getProjectionMatrix();
    glm::mat4 view = _cameras[activeCameraIndex]->getViewMatrix();

    _shader->use();
    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", view);
    _shader->setUniformMat4("model", _rifle->transform.getLocalMatrix());

    _rifle->draw();
}

void scene_and_shading::initShader() {
    const char* vsCode =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPosition;\n"
        "layout(location = 1) in vec3 aNormal;\n"

        "out vec3 worldPosition;\n"
        "out vec3 normal;\n"

        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"

        "void main() {\n"
        "    normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "    worldPosition = vec3(model * vec4(aPosition, 1.0f));\n"
        "    gl_Position = projection * view * vec4(worldPosition, 1.0f);\n"
        "}\n";

    const char* fsCode =
        "#version 330 core\n"
        "in vec3 worldPosition;\n"
        "in vec3 normal;\n"
        "out vec4 fragColor;\n"

        "void main() {\n"
        "    vec3 lightPosition = vec3(100.0f, 100.0f, 100.0f);\n"
        "    // ambient color\n"
        "    float ka = 0.1f;\n"
        "    vec3 objectColor = vec3(1.0f, 1.0f, 1.0f);\n"
        "    vec3 ambient = ka * objectColor;\n"
        "    // diffuse color\n"
        "    float kd = 0.8f;\n"
        "    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);\n"
        "    vec3 lightDirection = normalize(lightPosition - worldPosition);\n"
        "    vec3 diffuse = kd * lightColor * max(dot(normalize(normal), lightDirection), 0.0f);\n"
        "    fragColor = vec4(ambient + diffuse, 1.0f);\n"
        "}\n";

    _shader.reset(new GLSLProgram);
    _shader->attachVertexShader(vsCode);
    _shader->attachFragmentShader(fsCode);
    _shader->link();
}