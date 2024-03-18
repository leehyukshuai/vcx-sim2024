#include "Collision.h"

namespace VCX::Labs::RigidBody {
    BoxCollisionSystem::Contact::Contact(int _id1, int _id2, fcl::Vector3f _pos, fcl::Vector3f _normal, float _depth):
        id1(_id1), id2(_id2), pos(_pos[0], _pos[1], _pos[2]), normal(_normal[0], _normal[1], _normal[2]), depth(_depth) {}
    void BoxCollisionSystem::collisionDetect() {
        for (int i = 0; i < items.size(); ++i) {
            for (int j = i + 1; j < items.size(); ++j) {
                auto const & b0              = items[i];
                auto const & b1              = items[j];
                using CollisionGeometryPtr_t = std::shared_ptr<fcl::CollisionGeometry<float>>;
                CollisionGeometryPtr_t      box_geometry_A(new fcl::Box<float>(b0->dimension[0], b0->dimension[1], b0->dimension[2]));
                CollisionGeometryPtr_t      box_geometry_B(new fcl::Box<float>(b1->dimension[0], b1->dimension[1], b1->dimension[2]));
                Eigen::Quaternionf          b0q(b0->orientation.w, b0->orientation.x, b0->orientation.y, b0->orientation.z);
                Eigen::Quaternionf          b1q(b1->orientation.w, b1->orientation.x, b1->orientation.y, b1->orientation.z);
                Eigen::Vector3f             b0x(b0->position.x, b0->position.y, b0->position.z);
                Eigen::Vector3f             b1x(b1->position.x, b1->position.y, b1->position.z);
                fcl::CollisionObject<float> box_A(box_geometry_A, fcl::Transform3f(Eigen::Translation3f(b0x) * b0q));
                fcl::CollisionObject<float> box_B(box_geometry_B, fcl::Transform3f(Eigen::Translation3f(b1x) * b1q));
                // Compute collision -at most 8 contacts and return contact information.
                fcl::CollisionRequest<float> collisionRequest(8, true);
                fcl::CollisionResult<float>  collisionResult;
                fcl::collide(&box_A, &box_B, collisionRequest, collisionResult);
                if (! collisionResult.isCollision()) continue;
                ;
                std::vector<fcl::Contact<float>> fclContacts;
                collisionResult.getContacts(fclContacts);
                for (auto const & contact : fclContacts) { // You can decide whether define your own Contact
                    contacts.emplace_back(Contact(i, j, contact.pos, -contact.normal, contact.penetration_depth));
                }
            }
        }
    }
    void BoxCollisionSystem::collisionHandle() {
        // frictionless collision model
        std::vector<glm::vec3> totalVelocityChange;
        std::vector<glm::vec3> totalOmegaChange;
        std::vector<int>       multiCollisionCount;
        if (collisionMethod == METHOD_AVERAGE) {
            totalVelocityChange.resize(items.size());
            totalOmegaChange.resize(items.size());
            multiCollisionCount.resize(items.size());
        }
        for (auto const & contact : contacts) {
            auto  a    = items[contact.id1];
            auto  b    = items[contact.id2];
            auto  pai  = contact.pos - a->position;
            auto  pbi  = contact.pos - b->position;
            auto  vai  = glm::vec3(a->velocity + glm::cross(a->omega, contact.pos - a->position));
            auto  vbi  = glm::vec3(b->velocity + glm::cross(b->omega, contact.pos - b->position));
            float vrel = glm::dot(vai - vbi, contact.normal);
            if (vrel <= 0) {
                // when collision happens
                float  mai   = a->isStatic ? 0 : 1 / a->mass;
                float  mbi   = b->isStatic ? 0 : 1 / b->mass;
                auto & n     = contact.normal;
                auto   rota  = glm::toMat3(a->orientation);
                auto   Ia    = rota * a->inertia * glm::transpose(rota);
                auto   Iainv = a->isStatic ? glm::mat3(0) : glm::inverse(Ia);
                auto   rotb  = glm::toMat3(b->orientation);
                auto   Ib    = rotb * b->inertia * glm::transpose(rotb);
                auto   Ibinv = b->isStatic ? glm::mat3(0) : glm::inverse(Ib);
                // Empirical collision model based on c (coefficient of restitution)
                auto J = ((-(1.0f + c) * vrel) / (1.0f * mai + 1.0f * mbi + glm::dot(n, (Iainv * glm::cross(glm::cross(pai, n), pai) + Ibinv * glm::cross(glm::cross(pbi, n), pbi))))) * n;
                // apply impulse J
                if (collisionMethod == METHOD_NAIVE) {
                    a->velocity += J * mai;
                    b->velocity -= J * mbi;
                    a->omega += Iainv * glm::cross(pai, J);
                    b->omega += Ibinv * glm::cross(pbi, -J);
                } else if (collisionMethod == METHOD_AVERAGE) {
                    multiCollisionCount[contact.id1]++;
                    multiCollisionCount[contact.id2]++;
                    totalVelocityChange[contact.id1] += J * mai;
                    totalVelocityChange[contact.id2] -= J * mbi;
                    totalOmegaChange[contact.id1] += Iainv * glm::cross(pai, J);
                    totalOmegaChange[contact.id2] += Ibinv * glm::cross(pbi, -J);
                }
            } else {
                // or just contact, we move them apart
                if (a->isStatic && b->isStatic) continue;
                bool  anyStatic = (a->isStatic || b->isStatic);
                float ma        = a->mass;
                float mb        = b->mass;
                float partb     = anyStatic ? (a->isStatic ? 1 : 0) : ma / (ma + mb);
                float parta     = anyStatic ? (a->isStatic ? 0 : 1) : mb / (ma + mb);
                a->position += contact.normal * contact.depth * parta;
                b->position -= contact.normal * contact.depth * partb;
            }
        }
        if (collisionMethod == METHOD_AVERAGE) {
            for (int i = 0; i < items.size(); ++i) {
                if (multiCollisionCount[i]) {
                    // why not divided by (float) multiCollisionCount[i]?
                    items[i]->velocity += totalVelocityChange[i] / (float) multiCollisionCount[i];
                    items[i]->omega += totalOmegaChange[i] / (float) multiCollisionCount[i];
                }
            }
        }
        contacts.clear();
    }

} // namespace VCX::Labs::RigidBody