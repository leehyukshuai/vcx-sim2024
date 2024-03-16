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
                if (! collisionResult.isCollision()) return;
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
        // c represents coefficient of restitution
        static const float c = 0.6f;
        for (auto const & contact : contacts) {
            auto  a    = items[contact.id1];
            auto  b    = items[contact.id2];
            auto  pai  = contact.pos - a->position;
            auto  pbi  = contact.pos - b->position;
            auto  vai  = glm::vec3(a->velocity + glm::cross(a->omega, contact.pos - a->position));
            auto  vbi  = glm::vec3(b->velocity + glm::cross(b->omega, contact.pos - b->position));
            float vrel = glm::dot(vai - vbi, contact.normal);
            // when collision happens
            if (vrel < 0) {
                float  ma    = a->mass;
                float  mb    = b->mass;
                auto & n     = contact.normal;
                auto & pa    = a->position;
                auto & pb    = b->position;
                auto   rota  = glm::toMat3(a->orientation);
                auto   Ia    = rota * a->inertia * glm::transpose(rota);
                auto   Iainv = glm::inverse(Ia);
                auto   rotb  = glm::toMat3(b->orientation);
                auto   Ib    = rotb * b->inertia * glm::transpose(rotb);
                auto   Ibinv = glm::inverse(Ib);
                // Empirical collision model based on c (coefficient of restitution)
                auto J = ((-(1.0f + c) * vrel) / (1.0f / ma + 1.0f / mb + glm::dot(n, (Iainv * glm::cross(glm::cross(pa, n), pa) + Ibinv * glm::cross(glm::cross(pb, n), pb))))) * n;
                // apply impulse J
                a->velocity += J / ma;
                b->velocity += -J / mb;
                a->omega += Iainv * glm::cross(pai, J);
                b->omega += Ibinv * glm::cross(pbi, -J);
            }
        }
        contacts.clear();
    }

} // namespace VCX::Labs::RigidBody