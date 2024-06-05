#include "Car.h"

namespace VCX::Labs::OpenProj {
    Car::Car() { reset(); }
    std::vector<Object *> Car::objects() {
        return std::vector<Object *>({ &body, &wheels[0], &wheels[1], &wheels[2], &wheels[3] });
    }

    void Car::reset() {
        body.boxBody.reset();
        body.boxBody.density   = 0.3f;
        body.boxBody.dimension = glm::vec3(8, 3, 5);
        body.boxBody.position  = glm::vec3(0, 3.7, 0);
        body.renderItem.color  = glm::vec3(0.7f, 0.3f, 0.9f);
        body.initialize();

        wheels.clear();
        wheels.resize(4);
        wheels[0].sphereBody.position = glm::vec3(-3, 1.1, -1.5);
        wheels[1].sphereBody.position = glm::vec3(3, 1.1, 1.5);
        wheels[2].sphereBody.position = glm::vec3(-3, 1.1, 1.5);
        wheels[3].sphereBody.position = glm::vec3(3, 1.1, -1.5);
        for (auto & w : wheels) {
            w.sphereBody.radius = 1.0f;
            w.renderItem.color  = glm::vec3(0.2f, 0.2f, 0.3f);
            w.initialize();
        }
    }

    void Car::rigidify() {
        // TODO: shape match
        return;
    }

    void Car::move(bool keyMove[4]) {
        float rs = 10.0f;
        if (keyMove[0])
            for (auto & w : wheels) w.rigidBody->applyTorque({ 0, 0, -rs });
        if (keyMove[1])
            for (auto & w : wheels) w.rigidBody->applyTorque({ -rs, 0, 0 });
        if (keyMove[2])
            for (auto & w : wheels) w.rigidBody->applyTorque({ 0, 0, rs });
        if (keyMove[3])
            for (auto & w : wheels) w.rigidBody->applyTorque({ rs, 0, 0 });
    }

    Road::Road() { reset(); }
    void Road::reset() {
        sects.clear();
        sects.resize(3);
        sects[0].boxBody.position    = glm::vec3(-30, 7.5, 0);
        sects[0].boxBody.orientation = glm::quat(glm::vec3(0, 0, -glm::radians(30.0f)));
        sects[1].boxBody.position    = glm::vec3(0, -0.5, 0);
        sects[1].boxBody.orientation = glm::quat(glm::vec3(0, 0, 0));
        sects[2].boxBody.position    = glm::vec3(30, 7.5, 0);
        sects[2].boxBody.orientation = glm::quat(glm::vec3(0, 0, glm::radians(30.0f)));
        for (auto & w : sects) {
            w.boxBody.isStatic  = true;
            w.boxBody.dimension = glm::vec3(32, 1, 30);
            w.renderItem.color  = glm::vec3(0.2f, 1.0f, 0.3f);
            w.initialize();
        }
    }

    std::vector<Object *> Road::objects() {
        std::vector<Object *> ret;
        for (auto & s : sects) {
            ret.push_back(&s);
        }
        return ret;
    }

} // namespace VCX::Labs::OpenProj
