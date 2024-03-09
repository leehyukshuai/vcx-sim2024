#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

namespace VCX::Labs::RigidBody {
    class RigidBody {
        std::vector<std::pair<glm::vec3, glm::vec3>> forceList;
    public:
        float mass = 1.0f;
        glm::vec3 position = glm::vec3(0,0,0);
        glm::vec3 velocity = glm::vec3(0,0,0);
        glm::vec3 omega = glm::vec3(0,0,0);
        glm::quat orientation = glm::quat(1, 0, 0, 0);
        glm::mat3 inertia = glm::mat3(1);
        glm::vec3 totalTorque = glm::vec3(0,0,0);
        void apply(glm::vec3 force, glm::vec3 point = glm::vec3(0,0,0));
        void applyTorque(glm::vec3 torque);
        void update(float delta);
        void resetForces();
    };
}