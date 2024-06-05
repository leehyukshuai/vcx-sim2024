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

        // initialize relatives
        glm::vec3 c = calcCenter();
        relatives.clear();
        for (auto & o : objects()) {
            glm::vec3 r = o->rigidBody->position - c;
            relatives.push_back(r);
        }
    }

    void Car::rigidify() {
        // // TODO: shape match
        // auto objs = objects();
        // int  num  = objs.size();
        // auto c    = calcCenter();

        // glm::mat3 A, Al, Ar;
        // for (int i = 0; i < num; ++i) {
        //     auto & o  = objs[i];
        //     auto & yi = o->rigidBody->position;
        //     auto & ri = relatives[i];
        //     Al += glm::outerProduct(yi - c, ri);
        //     Ar += glm::outerProduct(ri, ri);
        // }
        // Ar = glm::inverse(Ar);
        // A  = Al * Ar;

        // auto polarDecomposition = [](const glm::mat3 & A) {
        //     // Convert glm::mat3 A to Eigen::Matrix3d
        //     Eigen::Matrix3f A_eigen = Eigen::Map<const Eigen::Matrix3f>(glm::value_ptr(A));

        //     // Compute SVD decomposition
        //     Eigen::JacobiSVD<Eigen::Matrix3f> svd(A_eigen, Eigen::ComputeFullU | Eigen::ComputeFullV);
        //     Eigen::Matrix3f                   U = svd.matrixU();
        //     Eigen::Matrix3f                   V = svd.matrixV();

        //     // Compute R = UV'
        //     Eigen::Matrix3f R = U * V.transpose();

        //     return glm::make_mat3(R.data());
        // };

        // auto R = polarDecomposition(A);
        // for (int i = 0; i < num; ++i) {
        //     auto & o  = objs[i];
        //     auto & ri = relatives[i];

        //     o->rigidBody->position = c + R * ri;
        // }
    }

    glm::vec3 Car::calcCenter() {
        glm::vec3 c;

        auto objs = objects();
        for (auto & o : objs) {
            c += o->rigidBody->position;
        }

        return c / (float) objs.size();
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
