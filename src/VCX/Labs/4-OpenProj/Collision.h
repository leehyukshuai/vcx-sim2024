#pragma once

#include <fcl/fcl.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace VCX::Labs::OpenProj {
    class Object;

    class CollisionItem {
    public:
        fcl::CollisionObject<float> collisionObject;

        CollisionItem();
        CollisionItem(const std::shared_ptr<fcl::CollisionGeometry<float>> & geom);
        void initialize(const std::shared_ptr<fcl::CollisionGeometry<float>> & geom);
        void updateBuffer(const glm::vec3 & translation, const glm::quat & rotation);
    };

    class CollisionSystem {
        struct Contact {
            int       id1;
            int       id2;
            glm::vec3 pos;
            glm::vec3 normal;
            float     depth;
            Contact(int _id1, int _id2, fcl::Vector3f _pos, fcl::Vector3f _normal, float _depth):
                id1(_id1), id2(_id2), pos(_pos[0], _pos[1], _pos[2]), normal(_normal[0], _normal[1], _normal[2]), depth(_depth) {}
        };
        std::vector<Contact> contacts;

    public:
        std::vector<Object *> items;

        float c = 0.6f; // coefficient of restitution
        float miu_N = 0.6f;
        float miu_T = 0.4f;

        enum CollisionHandleMethod { FRICTIONLESS_IMPULSE, FRICTIONAL_IMPULSE };
        CollisionHandleMethod collisionMethod = FRICTIONAL_IMPULSE;

        void collisionDetect();
        void collisionHandle();
    };

} // namespace VCX::Labs::OpenProj
