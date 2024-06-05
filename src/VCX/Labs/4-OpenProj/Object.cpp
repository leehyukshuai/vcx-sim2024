#include "Object.h"

namespace VCX::Labs::OpenProj {
    Box::Box(const glm::vec3 & dim):
        Object(&boxBody) {
        initialize(dim);
    }

    void Box::initialize(const glm::vec3 & dim) {
        boxBody.dimension = dim;
        initialize();
    }

    void Box::initialize() {
        glm::vec3 dim = boxBody.dimension;
        auto mesh = Mesh::generateBoxMesh(dim);
        renderItem.initialize(mesh);
        auto geom = std::make_shared<fcl::Box<float>>(dim[0], dim[1], dim[2]);
        collisionItem.initialize(geom);
        rigidBody->updateBuffer();
        updateBuffer();
    }

    Cylinder::Cylinder(float radius, float height, int precision):
        Object(&cylinderBody) {
        initialize(radius, height, precision);
    }

    void Cylinder::initialize(float radius, float height, int precision) {
        cylinderBody.radius = radius;
        cylinderBody.height = height;
        initialize();
    }

    void Cylinder::initialize(int precision) {
        float radius = cylinderBody.radius;
        float height = cylinderBody.height;
        auto mesh = Mesh::generateCylinderMesh(radius, height, precision);
        renderItem.initialize(mesh);
        auto geom = std::make_shared<fcl::Cylinder<float>>(radius, height);
        collisionItem.initialize(geom);
        rigidBody->updateBuffer();
        updateBuffer();
    }

    void Object::updateBuffer() {
        renderItem.updateBuffer(rigidBody->position, rigidBody->orientation);
        collisionItem.updateBuffer(rigidBody->position, rigidBody->orientation);
    }
} // namespace VCX::Labs::OpenProj
