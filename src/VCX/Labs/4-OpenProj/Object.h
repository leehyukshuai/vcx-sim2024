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
    };

    class Box : public Object {
    public:
        BoxBody boxBody;

        Box(glm::vec3 dim = glm::vec3(1, 1, 1));
        void initialize(const glm::vec3 & dim);
    };

    class Cylinder : public Object {
    public:
        CylinderBody cylinderBody;

        Cylinder(float radius = 0.5f, float height = 1.0f, int precision = 32);
        void initialize(float radius, float height, int precision);
    };

} // namespace VCX::Labs::OpenProj
