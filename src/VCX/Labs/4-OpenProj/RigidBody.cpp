#include "RigidBody.h"

namespace VCX::Labs::OpenProj {
    void RigidBody::apply(glm::vec3 force, glm::vec3 point) {
        forceList.emplace_back(force, point);
    }
    void RigidBody::applyTorque(glm::vec3 torque) {
        totalTorque += torque;
    }
    void RigidBody::update(float delta) {
        if (isStatic) {
            resetForces();
            return;
        }
        // translational state
        glm::vec3 totalForce = glm::vec3(0, 0, 0);
        for (auto forcePair : forceList) {
            glm::vec3 force = forcePair.first;
            totalForce += force;
        }
        velocity += totalForce / mass * delta;
        // rotational state
        auto rotationMatrix = glm::toMat3(orientation);
        for (auto forcePair : forceList) {
            glm::vec3 force  = forcePair.first;
            glm::vec3 point  = forcePair.second;
            glm::vec3 torque = glm::cross((rotationMatrix * point), force);
            totalTorque += torque;
        }
        angularMomentum += totalTorque * delta;
        auto rotatedInertia = rotationMatrix * inertia * glm::transpose(rotationMatrix);
        omega               = glm::inverse(rotatedInertia) * angularMomentum;
        // clear forces
        resetForces();
    }
    void RigidBody::move(float delta) {
        position += velocity * delta;
        orientation += ((delta / 2) * glm::quat(0, omega[0], omega[1], omega[2])) * orientation;
        orientation = glm::normalize(orientation);
    }
    void RigidBody::resetForces() {
        forceList.clear();
        totalTorque = glm::vec3(0, 0, 0);
    }
    void RigidBody::updateBuffer() {
        setMass();
        setInertia();
    }
    void RigidBody::reset() {
        isStatic        = false;
        mass            = 1.0f;
        density         = 1.0f;
        position        = glm::vec3(0, 0, 0);
        velocity        = glm::vec3(0, 0, 0);
        omega           = glm::vec3(0, 0, 0);
        angularMomentum = glm::vec3(0, 0, 0);
        orientation     = glm::quat(1, 0, 0, 0);
        inertia         = glm::mat3(1);
        resetForces();
    }
    void RigidBody::setInertia() {
        inertia[0][0] = 1.0f;
        inertia[1][1] = 1.0f;
        inertia[2][2] = 1.0f;
    }
    void RigidBody::setMass() {
        mass = 1.0f;
    }
    void RigidBody::applyTranslDamping(float translDampingFactor) {
        auto normalized = glm::normalize(velocity);
        if (! (std::isnan(normalized.x) || std::isnan(normalized.y) || std::isnan(normalized.z))) {
            apply(-translDampingFactor * glm::length2(velocity) * normalized);
        }
    }
    void RigidBody::applyRotateDamping(float rotateDampingFactor) {
        auto normalized = glm::normalize(omega);
        if (! (std::isnan(normalized.x) || std::isnan(normalized.y) || std::isnan(normalized.z))) {
            applyTorque(-rotateDampingFactor * glm::length2(omega) * normalized);
        }
    }
    void BoxBody::setInertia() {
        inertia[0][0] = mass / 12.0 * (dimension[1] * dimension[1] + dimension[2] * dimension[2]);
        inertia[1][1] = mass / 12.0 * (dimension[0] * dimension[0] + dimension[2] * dimension[2]);
        inertia[2][2] = mass / 12.0 * (dimension[0] * dimension[0] + dimension[1] * dimension[1]);
    }
    void BoxBody::setMass() {
        mass = density * dimension.x * dimension.y * dimension.z;
    }
    void BoxBody::reset() {
        dimension = glm::vec3(1, 1, 1);
        RigidBody::reset();
    }
    void CylinderBody::setInertia() {
        inertia[0][0] = mass * (3 * radius * radius + height * height) / 12.0;
        inertia[1][1] = mass * (radius * radius) / 2.0;
        inertia[2][2] = mass * (3 * radius * radius + height * height) / 12.0;
    }
    void CylinderBody::setMass() {
        float pi = glm::pi<float>();
        mass     = density * pi * radius * radius * height;
    }
    void CylinderBody::reset() {
        radius = 0.5f;
        height = 1.0f;
        RigidBody::reset();
    }
} // namespace VCX::Labs::OpenProj