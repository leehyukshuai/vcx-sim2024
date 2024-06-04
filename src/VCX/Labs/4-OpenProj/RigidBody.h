#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace VCX::Labs::OpenProj {
    class RigidBody {
    public:
        glm::vec3 position = glm::vec3(0, 0, 0);
        glm::vec3 velocity = glm::vec3(0, 0, 0);
    };

    class BoxBody : public RigidBody {
    public:
        glm::vec3 dimension = glm::vec3(1, 1, 1);
    };

    class CylinderBody : public RigidBody {
    public:
        float radius = 0.5f;
        float height = 1.0f;
    };
} // namespace VCX::Labs::OpenProj
