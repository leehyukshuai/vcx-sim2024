#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

namespace VCX::Labs::OpenProj {
    class Object;

    class Mesh {
    public:
        std::vector<glm::vec3>     positions;
        std::vector<std::uint32_t> triIndices;
        std::vector<std::uint32_t> lineIndices;

        static Mesh generateBoxMesh(glm::vec3 dim, int precision);
        static Mesh generateCylinderMesh(float radius, float height, int precision);
        static Mesh generateSphereMesh(float radius, int precision);
    };

    class RenderItem {
    public:
        glm::vec3 color { 0.5, 0.6, 0.7 };

        std::vector<glm::vec3> positions;

        Engine::GL::UniqueIndexedRenderItem lineItem;
        Engine::GL::UniqueIndexedRenderItem faceItem;

        RenderItem();
        RenderItem(const Mesh & mesh);
        void initialize(const Mesh & mesh);
        void updateBuffer(const glm::vec3 & translation, const glm::quat & rotation);
    };

    class RenderSystem {
    public:
        Engine::GL::UniqueProgram program;
        std::vector<Object *>     items;

        bool xrayed;

        RenderSystem();
        void draw(glm::mat4 cameraTransform);
    };

} // namespace VCX::Labs::OpenProj
