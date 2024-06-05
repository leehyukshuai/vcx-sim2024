#pragma once

#include <fcl/fcl.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace VCX::Labs::OpenProj {
    class Object;

    class CollisionItem {
    public:
        int mask = 0; // only objects with same mask can collide

        fcl::CollisionObject<float> collisionObject;

        CollisionItem();
        CollisionItem(const std::shared_ptr<fcl::CollisionGeometry<float>> & geom, int mask = 0);
        void initialize(const std::shared_ptr<fcl::CollisionGeometry<float>> & geom, int mask = 0);
        void updateBuffer(const glm::vec3 & translation, const glm::quat & rotation);
    };

    class CollisionSystem {
        struct Contact {
            Object *  p1;
            Object *  p2;
            glm::vec3 pos;
            glm::vec3 normal;
            float     depth;
            Contact(Object * _p1, Object * _p2, fcl::Vector3f _pos, fcl::Vector3f _normal, float _depth):
                p1(_p1), p2(_p2), pos(_pos[0], _pos[1], _pos[2]), normal(_normal[0], _normal[1], _normal[2]), depth(_depth) {}
        };
        std::vector<Contact> contacts;

    public:
        std::vector<Object *> items;

        float c     = 0.6f; // FRICTIONLESS_IMPULSE
        float miu_N = 0.6f; // FRICTIONAL_IMPULSE
        float miu_T = 3.0f; // FRICTIONAL_IMPULSE

        enum CollisionHandleMethod { FRICTIONLESS_IMPULSE,
                                     FRICTIONAL_IMPULSE };
        CollisionHandleMethod collisionMethod = FRICTIONAL_IMPULSE;

        void collisionDetect();
        void collisionHandle();
    };

} // namespace VCX::Labs::OpenProj
