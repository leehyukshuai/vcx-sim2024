#include "Collision.h"

namespace VCX::Labs::OpenProj {
    CollisionItem::CollisionItem():
        collisionObject(std::make_shared<fcl::Box<float>>(1.0, 1.0, 1.0)) {
    }

    CollisionItem::CollisionItem(const std::shared_ptr<fcl::CollisionGeometry<float>> & geom):
        collisionObject(fcl::CollisionObject<float>(geom)) {
    }

    void CollisionItem::initialize(const std::shared_ptr<fcl::CollisionGeometry<float>> & geom) {
        collisionObject = fcl::CollisionObject<float>(geom);
    }

    void CollisionItem::updateBuffer(const glm::vec3 & translation, const glm::quat & rotation) {
        fcl::Quaternionf rot(rotation.w, rotation.x, rotation.y, rotation.z);
        fcl::Vector3f    trans(translation.x, translation.y, translation.z);
        collisionObject.setQuatRotation(rot);
        collisionObject.setTranslation(trans);
    }

}