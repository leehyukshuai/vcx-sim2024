#include "Object.h"

namespace VCX::Labs::OpenProj {
    void Object::updateRenderBuffer() {
        renderItem.updateBuffer(rigidBody->position, rigidBody->orientation);
    }

    void Object::updateCollisionBuffer() {
        collisionItem.updateBuffer(rigidBody->position, rigidBody->orientation);
    }
    
    void Object::updateBuffer() {
        updateRenderBuffer();
        updateCollisionBuffer();
    }

    Box::Box(const glm::vec3 & dim):
        Object(&boxBody) {
        boxBody.dimension = dim;
        initialize();
    }

    void Box::initialize() {
        glm::vec3 dim  = boxBody.dimension;
        auto      mesh = Mesh::generateBoxMesh(dim, 16);
        renderItem.initialize(mesh);
        auto geom = std::make_shared<fcl::Box<float>>(dim[0], dim[1], dim[2]);
        geom.get()->setUserData(this);
        collisionItem.initialize(geom);
        rigidBody->updateBuffer();
        updateBuffer();
    }

    Cylinder::Cylinder(float radius, float height):
        Object(&cylinderBody) {
        cylinderBody.radius = radius;
        cylinderBody.height = height;
        initialize();
    }

    void Cylinder::initialize() {
        float radius = cylinderBody.radius;
        float height = cylinderBody.height;
        auto  mesh   = Mesh::generateCylinderMesh(radius, height, 16);
        renderItem.initialize(mesh);
        auto geom = std::make_shared<fcl::Cylinder<float>>(radius, height);
        geom.get()->setUserData(this);
        collisionItem.initialize(geom);
        rigidBody->updateBuffer();
        updateBuffer();
    }

    Sphere::Sphere(float radius):
        Object(&sphereBody) {
        sphereBody.radius = radius;
        initialize();
    }

    void Sphere::initialize() {
        float radius = sphereBody.radius;
        auto  mesh   = Mesh::generateSphereMesh(radius, 32);
        renderItem.initialize(mesh);
        auto geom = std::make_shared<fcl::Sphere<float>>(radius);
        geom.get()->setUserData(this);
        collisionItem.initialize(geom);
        rigidBody->updateBuffer();
        updateBuffer();
    }
} // namespace VCX::Labs::OpenProj
