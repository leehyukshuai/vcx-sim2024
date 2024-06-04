#include "Object.h"

namespace VCX::Labs::OpenProj {
    Object::Object(RigidBody * bodyPtr):
        rigidBody(bodyPtr) {}

    Box::Box(glm::vec3 dim):
        Object(&boxBody) {
        initialize(dim);
    }

    void Box::initialize(const glm::vec3 & dim) {
        auto mesh = Mesh::generateBoxMesh(dim);
        renderItem.initialize(mesh);
        auto geom = std::make_shared<fcl::Box<float>>(dim[0], dim[1], dim[2]);
        collisionItem.initialize(geom);
    }

    Cylinder::Cylinder(float radius, float height, int precision):
        Object(&cylinderBody) {
        initialize(radius, height, precision);
    }

    void Cylinder::initialize(float radius, float height, int precision) {
        auto mesh = Mesh::generateCylinderMesh(radius, height, precision);
        renderItem.initialize(mesh);
        auto geom = std::make_shared<fcl::Cylinder<float>>(radius, height);
        collisionItem.initialize(geom);
    }
} // namespace VCX::Labs::OpenProj
