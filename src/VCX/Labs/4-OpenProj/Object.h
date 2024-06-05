#pragma once

#include "Render.h"
#include "RigidBody.h"
#include "Collision.h"

namespace VCX::Labs::OpenProj {
    class Object {
    public:
        RigidBody *   rigidBody = nullptr;
        RenderItem    renderItem;
        CollisionItem collisionItem;

        Object(RigidBody * bodyPtr):
            rigidBody(bodyPtr) {}
        
        void updateBuffer();
        virtual void initialize() = 0;
    };

    class Box : public Object {
    public:
        BoxBody boxBody;

        Box(const glm::vec3 & dim = glm::vec3(1, 1, 1));
        void initialize() override;
    };

    class Cylinder : public Object {
    public:
        CylinderBody cylinderBody;

        Cylinder(float radius = 0.5f, float height = 1.0f);
        void initialize() override;
    };

    class Sphere : public Object {
    public:
        SphereBody sphereBody;

        Sphere(float radius = 0.5f);
        void initialize() override;
    };

} // namespace VCX::Labs::OpenProj
