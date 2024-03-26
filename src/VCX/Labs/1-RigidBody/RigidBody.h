#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "Engine/GL/RenderItem.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

namespace VCX::Labs::RigidBody {
    class RigidBody {
        std::vector<std::pair<glm::vec3, glm::vec3>> forceList;
        glm::vec3                                    totalTorque = glm::vec3(0, 0, 0);

    public:
        bool         isStatic        = false;
        float        mass            = 1.0f;
        float        density         = 1.0f;
        glm::vec3    position        = glm::vec3(0, 0, 0);
        glm::vec3    velocity        = glm::vec3(0, 0, 0);
        glm::vec3    omega           = glm::vec3(0, 0, 0);
        glm::vec3    angulayMomentum = glm::vec3(0, 0, 0);
        glm::quat    orientation     = glm::quat(1, 0, 0, 0);
        glm::mat3    inertia         = glm::mat3(1);
        virtual void setInertia();
        virtual void setMass();
        virtual void applyTranslDamping(float translDampingFactor);
        virtual void applyRotateDamping(float rotateDampingFactor);
        glm::mat3    getRotatedInertia() const;
        void         setAngularMomentumByOmega(glm::vec3 newOmega);
        void         apply(glm::vec3 force, glm::vec3 point = glm::vec3(0, 0, 0));
        void         applyTorque(glm::vec3 torque);
        // update velocity and omega
        void update(float delta);
        // update position and orientation
        void move(float delta);
        void resetForces();
    };

    class Box : public RigidBody {
    public:
        glm::vec3 dimension = glm::vec3(1, 1, 1);
        void      setInertia() override;
        void      setMass() override;
    };

    class BoxRenderItem {
    public:
        Box                                 box;
        glm::vec3                           color { 0.5, 0.6, 0.7 };
        Engine::GL::UniqueIndexedRenderItem lineItem;
        Engine::GL::UniqueIndexedRenderItem faceItem;
        BoxRenderItem();
        // reset the vertex buffer info
        void updateBuffer();
    };

} // namespace VCX::Labs::RigidBody