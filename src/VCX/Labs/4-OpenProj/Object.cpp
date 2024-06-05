#include "Object.h"

namespace VCX::Labs::OpenProj {
    void Object::updateBuffer() {
        renderItem.updateBuffer(rigidBody->position, rigidBody->orientation);
        collisionItem.updateBuffer(rigidBody->position, rigidBody->orientation);
    }

    Box::Box(const glm::vec3 & dim):
        Object(&boxBody) {
        initialize(dim);
    }

    void Box::initialize(const glm::vec3 & dim) {
        boxBody.dimension = dim;
        initialize();
    }

    void Box::initialize() {
        glm::vec3 dim  = boxBody.dimension;
        auto      mesh = Mesh::generateBoxMesh(dim);
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
        initialize(precision);
    }

    void Cylinder::initialize(int precision) {
        float radius = cylinderBody.radius;
        float height = cylinderBody.height;
        auto  mesh   = Mesh::generateCylinderMesh(radius, height, precision);
        renderItem.initialize(mesh);
        auto geom = std::make_shared<fcl::Cylinder<float>>(radius, height);
        collisionItem.initialize(geom);
        rigidBody->updateBuffer();
        updateBuffer();
    }

    Sphere::Sphere(float radius, int precision):
        Object(&sphereBody) {
        initialize(precision);
    }

    void Sphere::initialize(float radius, int precision) {
        sphereBody.radius = radius;
        initialize(precision);
    }

    void Sphere::initialize(int precision) {
        float radius = sphereBody.radius;
        auto  mesh   = Mesh::generateSphereMesh(radius, precision);
        renderItem.initialize(mesh);
        auto geom = std::make_shared<fcl::Sphere<float>>(radius);
        collisionItem.initialize(geom);
        rigidBody->updateBuffer();
        updateBuffer();
    }
} // namespace VCX::Labs::OpenProj
