#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

namespace VCX::Labs::OpenProj {
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
        glm::vec3    angularMomentum = glm::vec3(0, 0, 0);
        glm::quat    orientation     = glm::quat(1, 0, 0, 0);
        glm::mat3    inertia         = glm::mat3(1);
        virtual void setInertia();
        virtual void setMass();
        virtual void reset();
        void         resetForces();
        void         updateBuffer();
        virtual void applyTranslDamping(float translDampingFactor);
        virtual void applyRotateDamping(float rotateDampingFactor);
        void         apply(glm::vec3 force, glm::vec3 point = glm::vec3(0, 0, 0));
        void         applyTorque(glm::vec3 torque);
        // update velocity and omega
        void update(float delta);
        // update position and orientation
        void move(float delta);
    };

    class BoxBody : public RigidBody {
    public:
        glm::vec3 dimension = glm::vec3(1, 1, 1);
        void      setInertia() override;
        void      setMass() override;
        void      reset() override;
    };

    class CylinderBody : public RigidBody {
    public:
        float radius = 0.5f;
        float height = 1.0f;
        void  setInertia() override;
        void  setMass() override;
        void  reset() override;
    };
} // namespace VCX::Labs::OpenProj
