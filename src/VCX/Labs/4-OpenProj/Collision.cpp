#include "Collision.h"
#include "Object.h"

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

    void CollisionSystem::collisionDetect() {
        for (int i = 0; i < items.size(); ++i) {
            for (int j = i + 1; j < items.size(); ++j) {
                auto const & b0 = items[i]->collisionItem.collisionObject;
                auto const & b1 = items[j]->collisionItem.collisionObject;
                // Compute collision -at most 8 contacts and return contact information.
                fcl::CollisionRequest<float> collisionRequest(8, true);
                fcl::CollisionResult<float>  collisionResult;
                fcl::collide(&b0, &b1, collisionRequest, collisionResult);
                if (! collisionResult.isCollision()) continue;
                std::vector<fcl::Contact<float>> fclContacts;
                collisionResult.getContacts(fclContacts);
                for (auto const & contact : fclContacts) { // You can decide whether define your own Contact
                    contacts.emplace_back(Contact(i, j, contact.pos, -contact.normal, contact.penetration_depth));
                }
            }
        }
    }

    void CollisionSystem::collisionHandle() {
        for (auto const & contact : contacts) {
            auto  a      = items[contact.id1]->rigidBody;
            auto  b      = items[contact.id2]->rigidBody;
            auto  pai    = contact.pos - a->position;
            auto  pbi    = contact.pos - b->position;
            auto  vai    = glm::vec3(a->velocity + glm::cross(a->omega, contact.pos - a->position));
            auto  vbi    = glm::vec3(b->velocity + glm::cross(b->omega, contact.pos - b->position));
            auto  vrel   = vai - vbi;
            float vrel_n = glm::dot(vrel, contact.normal);
            if (vrel_n <= 0) {
                glm::vec3 J;
                float     mai   = a->isStatic ? 0 : 1 / a->mass;
                float     mbi   = b->isStatic ? 0 : 1 / b->mass;
                auto &    n     = contact.normal;
                auto      rota  = glm::toMat3(a->orientation);
                auto      Ia    = rota * a->inertia * glm::transpose(rota);
                auto      Iainv = a->isStatic ? glm::mat3(0) : glm::inverse(Ia);
                auto      rotb  = glm::toMat3(b->orientation);
                auto      Ib    = rotb * b->inertia * glm::transpose(rotb);
                auto      Ibinv = b->isStatic ? glm::mat3(0) : glm::inverse(Ib);
                if (collisionMethod == FRICTIONLESS_IMPULSE) {
                    J = ((-(1.0f + c) * vrel) / (1.0f * mai + 1.0f * mbi + glm::dot(n, (Iainv * glm::cross(glm::cross(pai, n), pai) + Ibinv * glm::cross(glm::cross(pbi, n), pbi))))) * n;
                }
                if (collisionMethod == FRICTIONAL_IMPULSE) {
                    auto      vreln     = vrel_n * n;
                    auto      vrelt     = vrel - vreln;
                    float     A         = std::max(1 - miu_T * (1 + miu_N) * glm::length(vreln) / glm::length(vrelt), 0.0f);
                    auto      vreln_new = -miu_N * vreln;
                    auto      vrelt_new = A * vrelt;
                    auto      vrel_new  = vreln_new + vrelt_new;

                    auto crossProductMatrix = [](const glm::vec3 & v) { return glm::mat3(0, -v.z, v.y, v.z, 0, -v.x, -v.y, v.x, 0); };

                    glm::mat3 pai_m = crossProductMatrix(pai);
                    glm::mat3 pbi_m = crossProductMatrix(pbi);
                    glm::mat3 Ka    = a->isStatic ? glm::mat3(0) : (mai * glm::mat3(1.0f) - pai_m * Iainv * pai_m);
                    glm::mat3 Kb    = b->isStatic ? glm::mat3(0) : (mbi * glm::mat3(1.0f) - pbi_m * Ibinv * pbi_m);
                    glm::mat3 Kinv  = glm::inverse(Ka + Kb);

                    J = Kinv * (vrel_new - vrel);
                }
                // apply impulse J
                a->velocity += J * mai;
                b->velocity -= J * mbi;
                a->angularMomentum += glm::cross(pai, J);
                b->angularMomentum += glm::cross(pbi, -J);
            }
            else {
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
        contacts.clear();
    }

} // namespace VCX::Labs::OpenProj