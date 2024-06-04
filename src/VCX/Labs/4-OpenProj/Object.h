#pragma once

#include "RigidBody.h"
#include "Render.h"
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

        Box():
            Object(&boxBody) {}

        Box(const glm::vec3 & dim):
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

        Cylinder():
            Object(&cylinderBody) {}

        Cylinder(float radius, float height):
            Object(&cylinderBody) {
            initialize(radius, height);
        }

        void initialize(float radius, float height) {
            auto mesh = Mesh::generateCylinderMesh(radius, height);
            renderItem.initialize(mesh);
            auto geom = std::make_shared<fcl::Cylinder<float>>(radius, height);
            collisionItem.initialize(geom);
        }
    };

} // namespace VCX::Labs::OpenProj
