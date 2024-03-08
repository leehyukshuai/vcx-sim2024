#include "RigidBody.h"

namespace VCX::Labs::RigidBody {
    void RigidBody::apply(glm::vec3 force, glm::vec3 point) {
        forceList.emplace_back(force, point);
    }
    void RigidBody::applyTorque(glm::vec3 torque) {
        totalTorque += torque;
    }
    void RigidBody::update(float delta) {
        // translational state
        glm::vec3 totalForce = glm::vec3(0,0,0);
        for (auto forcePair : forceList) {
            glm::vec3 force = forcePair.first;
            totalForce += force;
        }
        velocity += totalForce / mass * delta;
        position += velocity * delta;
        // rotational state
        auto rotationMatrix = glm::toMat3(orientation);
        for (auto forcePair : forceList) {
            glm::vec3 force = forcePair.first;
            glm::vec3 point = forcePair.second;
            glm::vec3 torque = glm::cross((rotationMatrix * point), force);
            totalTorque += torque;
        }
        auto rotatedInertia = rotationMatrix * inertia * glm::transpose(rotationMatrix);
        omega += delta * glm::inverse(rotatedInertia) * totalTorque;
        orientation += ((delta / 2) * glm::quat(0, omega[0], omega[1], omega[2])) * orientation;
        orientation = glm::normalize(orientation);
        // clear forces
        forceList.clear();
        totalTorque = glm::vec3(0,0,0);
    }
}