#include "Car.h"
#include <Eigen/Dense>
#include <glm/gtc/type_ptr.hpp>

namespace VCX::Labs::OpenProj {
    Car::Car() { reset(); }

    std::vector<Object *> Car::objects() {
        return std::vector<Object *>({ &body, &wheels[0], &wheels[1], &wheels[2], &wheels[3] });
    }

    void Car::reset() {
        body.boxBody.reset();
        body.boxBody.density   = 0.1f;
        body.boxBody.dimension = glm::vec3(8, 3, 5);
        body.boxBody.position  = glm::vec3(0, 3.7, 0);
        body.renderItem.color  = glm::vec3(0.7f, 0.3f, 0.9f);
        body.initialize();

        wheels.clear();
        wheels.resize(4);
        wheels[0].sphereBody.position = glm::vec3(-3, 1.1, -1.5);
        wheels[1].sphereBody.position = glm::vec3(3, 1.1, -1.5);
        wheels[2].sphereBody.position = glm::vec3(-3, 1.1, 1.5);
        wheels[3].sphereBody.position = glm::vec3(3, 1.1, 1.5);
        for (auto & w : wheels) {
            w.sphereBody.radius = 1.0f;
            w.renderItem.color  = glm::vec3(0.2f, 0.5f, 0.8f);
            w.initialize();
        }

        // initialize distMatrix
        distMatrix.clear();
        auto objs = objects();
        int  num  = objs.size();
        for (auto & o : objs) {
            std::vector<float> distVector;
            for (auto & p : objs) {
                auto & ox = o->rigidBody->position;
                auto & px = p->rigidBody->position;
                distVector.push_back(glm::distance(ox, px));
            }
            distMatrix.push_back(std::move(distVector));
        }

        // set mask
        for (auto & o : objs) {
            o->collisionItem.mask = 1;
        }
    }

    void Car::rigidify() {
        std::vector<glm::vec3> global(4);
        global[0] = glm::vec3(-3, -1.5, -1.5);
        global[1] = glm::vec3(3, -1.5, -1.5);
        global[2] = glm::vec3(-3, -1.5, 1.5);
        global[3] = glm::vec3(3, -1.5, 1.5);
        std::vector<glm::vec3> local(4);
        local[0] = glm::vec3(-1, 0, -1);
        local[1] = glm::vec3(-1, 0, 1);
        local[2] = glm::vec3(1, 0, 1);
        local[3] = glm::vec3(1, 0, -1);

        for (int k = 0; k < 4; ++k) {
            auto & wheel = wheels[k];
            for (int i = 0; i < 4; ++i) {
                glm::vec3 r = global[k] + local[i];

                auto  ax    = body.boxBody.position + glm::rotate(body.boxBody.orientation, r);
                auto  bx    = wheel.sphereBody.position;
                float dist  = glm::distance(ax, bx);
                float delta = glm::sqrt(3) - dist;
                if (delta > 0) delta = glm::max(glm::sqrt(delta), delta * delta);
                else delta = -glm::max(glm::sqrt(-delta), delta * delta);
                float scale = stiffness * delta;
                auto  rel   = bx - ax;
                auto  reln  = glm::normalize(rel);
                auto  force = scale * reln;

                wheel.sphereBody.apply(force);
                body.boxBody.apply(-force, r);
            }
        }
    }

    void Car::move(bool keyMove[5]) {
        glm::vec3 forward(0, 0, -torque);
        glm::vec3 backward(0, 0, torque);
        glm::vec3 leftward(-torque, 0, 0);
        glm::vec3 rightward(torque, 0, 0);
        forward   = glm::rotate(body.boxBody.orientation, forward);
        backward  = glm::rotate(body.boxBody.orientation, backward);
        leftward  = glm::rotate(body.boxBody.orientation, leftward);
        rightward = glm::rotate(body.boxBody.orientation, rightward);

        // keyMove: W+A+S+D+Brake
        if (keyMove[0])
            for (auto & w : wheels) w.rigidBody->applyTorque(forward);
        if (keyMove[1]) {
            for (int i = 0; i < wheels.size(); ++i) {
                auto & w = wheels[i];
                if (i % 2) {
                    w.rigidBody->applyTorque(leftward);
                } else {
                    w.rigidBody->applyTorque(rightward);
                }
            }
        }
        if (keyMove[2])
            for (auto & w : wheels) w.rigidBody->applyTorque(backward);
        if (keyMove[3]) {
            for (int i = 0; i < wheels.size(); ++i) {
                auto & w = wheels[i];
                if (i % 2) {
                    w.rigidBody->applyTorque(rightward);
                } else {
                    w.rigidBody->applyTorque(leftward);
                }
            }
        }
        if (keyMove[4]) {
            for (auto & w : wheels) {
                auto brakeTorque = -brake * w.rigidBody->angularMomentum;
                w.rigidBody->applyTorque(brakeTorque);
            }
        }
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
            w.boxBody.dimension = glm::vec3(32, 1, 64);
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
