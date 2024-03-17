#pragma once
#include "RigidBody.h"
#include <fcl/fcl.h>

namespace VCX::Labs::RigidBody {
    class BoxCollisionSystem {
        struct Contact {
            int       id1;
            int       id2;
            glm::vec3 pos;
            glm::vec3 normal;
            float     depth;
            Contact(int _id1, int _id2, fcl::Vector3f _pos, fcl::Vector3f _normal, float _depth);
        };
        std::vector<Contact> contacts;

    public:
        enum CollisionHandleMethod { METHOD_NAIVE,
                                     METHOD_AVERAGE };
        std::vector<Box *>    items;
        float                 c               = 0.6f; // coefficient of restitution
        CollisionHandleMethod collisionMethod = METHOD_NAIVE;
        void                  collisionDetect();
        void                  collisionHandle();
    };
} // namespace VCX::Labs::RigidBody