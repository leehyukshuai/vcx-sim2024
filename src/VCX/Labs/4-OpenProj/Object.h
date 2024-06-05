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
    };

    class Box : public Object {
    public:
        BoxBody boxBody;

        Box(const glm::vec3 & dim = glm::vec3(1, 1, 1));
        void initialize(const glm::vec3 & dim);
        void initialize();
    };

    class Cylinder : public Object {
    public:
        CylinderBody cylinderBody;

        Cylinder(float radius = 0.5f, float height = 1.0f, int precision = 32);
        void initialize(float radius, float height, int precision = 32);
        void initialize(int precision = 32);
    };

    class Sphere : public Object {
    public:
        SphereBody sphereBody;

        Sphere(float radius = 0.5f, int precision = 32);
        void initialize(float radius, int precision = 32);
        void initialize(int precision = 32);
    };

} // namespace VCX::Labs::OpenProj
