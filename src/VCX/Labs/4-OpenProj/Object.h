#pragma once

#include "Collision.h"
#include "Render.h"
#include "RigidBody.h"

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

        Box(glm::vec3 dim = glm::vec3(1, 1, 1)):
            Object(&boxBody) {
            initialize(dim);
        }

        void initialize(const glm::vec3 & dim) {
            auto mesh = Mesh::generateBoxMesh(dim);
            renderItem.initialize(mesh);
            auto geom = std::make_shared<fcl::Box<float>>(dim[0], dim[1], dim[2]);
            collisionItem.initialize(geom);
        }
    };

    class Cylinder : public Object {
    public:
        CylinderBody cylinderBody;

        Cylinder(float radius = 0.5f, float height = 1.0f, int precision = 32):
            Object(&cylinderBody) {
            initialize(radius, height, precision);
        }

        void initialize(float radius, float height, int precision) {
            auto mesh = Mesh::generateCylinderMesh(radius, height, precision);
            renderItem.initialize(mesh);
            auto geom = std::make_shared<fcl::Cylinder<float>>(radius, height);
            collisionItem.initialize(geom);
        }
    };

} // namespace VCX::Labs::OpenProj
