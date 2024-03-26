#include "RigidBody.h"

namespace VCX::Labs::RigidBody {
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
        angulayMomentum += totalTorque * delta;
        auto rotatedInertia = rotationMatrix * inertia * glm::transpose(rotationMatrix);
        omega = glm::inverse(rotatedInertia) * angulayMomentum;
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
        if (!(std::isnan(normalized.x) || std::isnan(normalized.y) || std::isnan(normalized.z))) {
            apply(-translDampingFactor * glm::length2(velocity) * normalized);
        }
    }
    void RigidBody::applyRotateDamping(float rotateDampingFactor) {
        auto normalized = glm::normalize(omega);
        if (!(std::isnan(normalized.x) || std::isnan(normalized.y) || std::isnan(normalized.z))) {
            applyTorque(-rotateDampingFactor * glm::length2(omega) * normalized);
        }
    }
    glm::mat3 RigidBody::getRotatedInertia() const {
        auto rotationMatrix = glm::toMat3(orientation);
        auto rotatedInertia = rotationMatrix * inertia * glm::transpose(rotationMatrix);
        return rotatedInertia;
    }
    void RigidBody::setAngularMomentumByOmega(glm::vec3 newOmega) {
        angulayMomentum = getRotatedInertia() * newOmega;
    }
    void Box::setInertia() {
        inertia[0][0] = mass / 12.0 * (dimension[1] * dimension[1] + dimension[2] * dimension[2]);
        inertia[1][1] = mass / 12.0 * (dimension[0] * dimension[0] + dimension[2] * dimension[2]);
        inertia[2][2] = mass / 12.0 * (dimension[0] * dimension[0] + dimension[1] * dimension[1]);
    }
    void Box::setMass() {
        mass = density * dimension.x * dimension.y * dimension.z;
    }
    BoxRenderItem::BoxRenderItem():
        faceItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Triangles),
        lineItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Lines) {
        const static std::vector<std::uint32_t> tri_index  = { 0, 1, 2, 0, 2, 3, 1, 4, 0, 1, 4, 5, 1, 6, 5, 1, 2, 6, 2, 3, 7, 2, 6, 7, 0, 3, 7, 0, 4, 7, 4, 5, 6, 4, 6, 7 };
        const static std::vector<std::uint32_t> line_index = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };
        faceItem.UpdateElementBuffer(tri_index);
        lineItem.UpdateElementBuffer(line_index);
    }
    void BoxRenderItem::updateBuffer() {
        float x             = box.dimension[0] / 2.0;
        float y             = box.dimension[1] / 2.0;
        float z             = box.dimension[2] / 2.0;
        auto  VertsPosition = std::vector<glm::vec3>({
            {-x, -y,  z},
            { x, -y,  z},
            { x, -y, -z},
            {-x, -y, -z},
            {-x,  y,  z},
            { x,  y,  z},
            { x,  y, -z},
            {-x,  y, -z},
        });
        for (auto & vert : VertsPosition) {
            vert = glm::rotate(box.orientation, vert);
            vert = box.position + vert;
        }
        auto span_bytes = Engine::make_span_bytes<glm::vec3>(VertsPosition);
        lineItem.UpdateVertexBuffer("position", span_bytes);
        faceItem.UpdateVertexBuffer("position", span_bytes);
    }
} // namespace VCX::Labs::RigidBody