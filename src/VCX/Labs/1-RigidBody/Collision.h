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
        std::vector<Box *> items;
        void               collisionDetect();
        void               collisionHandle();
    };
} // namespace VCX::Labs::RigidBody