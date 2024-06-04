#pragma once

#include <fcl/fcl.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace VCX::Labs::OpenProj {
    class CollisionItem {
    public:
        fcl::CollisionObject<float> collisionObject;

        CollisionItem();
        CollisionItem(const std::shared_ptr<fcl::CollisionGeometry<float>> & geom);
        void initialize(const std::shared_ptr<fcl::CollisionGeometry<float>> & geom);
        void updateBuffer(const glm::vec3 & translation, const glm::quat & rotation);
    };
} // namespace VCX::Labs::OpenProj
