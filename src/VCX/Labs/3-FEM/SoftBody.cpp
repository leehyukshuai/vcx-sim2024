#include "SoftBody.h"
#include <Eigen/Eigen>

void VCX::Labs::FEM::SoftBody::initialize(const std::vector<glm::vec3> & position, const std::vector<glm::uvec3> & faces, const std::vector<glm::uvec4> & tetras, glm::mat4 transform) {
    int n = position.size();
    _rest.clear();
    _rest.reserve(n);
    _position.clear();
    _position.reserve(n);
    for (auto & pos : position) {
        auto p = glm::vec3(transform * glm::vec4(pos, 1.0));
        _rest.push_back(p);
        _position.push_back(p);
    }
    _faces  = faces;
    _tetras = tetras;

    _velocity.assign(n, glm::vec3());
    _fixed.assign(n, false);

    _lame = { 500.0f, 500.0f }; // TODO: modify lame parameters!
}

void VCX::Labs::FEM::SoftBody::initialize(const std::vector<glm::vec3> & position, const std::vector<glm::uvec3> & faces, const std::vector<glm::uvec4> & tetras, const std::vector<bool> & fixed, glm::mat4 transform) {
    initialize(position, faces, tetras, transform);
    _fixed = fixed;
}

int VCX::Labs::FEM::SoftBody::findnNearest(const glm::vec3 & eye, const glm::vec3 & dir) {
    // TODO
    return 0;
}

void VCX::Labs::FEM::SoftBody::applyConstraint(const std::function<glm::vec3(glm::vec3, glm::vec3, int)> & constraint) {
    _constraints.push_back(constraint);
}

std::pair<float, float> VCX::Labs::FEM::SoftBody::getLame() const {
    return _lame;
}

void VCX::Labs::FEM::SoftBody::setLame(std::pair<float, float> lame) {
    _lame = lame;
}

void VCX::Labs::FEM::SoftBody::update(float dt) {
    int n = _position.size();
    // clear force
    _force.assign(n, glm::vec3());
    // solve inner
    solveInner();
    // solve outer
    for (auto & constraint : _constraints) {
        for (int i = 0; i < n; ++i) {
            _force[i] += constraint(_position[i], _velocity[i], i);
        }
    }
    _constraints.clear();
    // apply force
    float m = 1.0f;
    for (int i = 0; i < n; ++i) {
        if (_fixed[i]) continue;
        _velocity[i] += dt * _force[i] / m;
        _position[i] += dt * _velocity[i];
    }
}

void VCX::Labs::FEM::SoftBody::solveInner() {
    for (auto & tetra : _tetras) {
        unsigned  i0 = tetra[0], i1 = tetra[1], i2 = tetra[2], i3 = tetra[3];
        glm::vec3 x0 = _position[i0], x1 = _position[i1], x2 = _position[i2], x3 = _position[i3];
        glm::vec3 X0 = _rest[i0], X1 = _rest[i1], X2 = _rest[i2], X3 = _rest[i3];
        glm::vec3 x10 = x1 - x0, x20 = x2 - x0, x30 = x3 - x0;
        glm::vec3 X10 = X1 - X0, X20 = X2 - X0, X30 = X3 - X0;

        auto E   = glm::transpose(glm::mat3(X10, X20, X30));
        auto e   = glm::transpose(glm::mat3(x10, x20, x30));
        auto F   = e * glm::inverse(E);
        auto G   = (glm::transpose(F) * F - glm::mat3(1.0f)) / 2.0f;
        auto S   = 2 * _lame.second * G + _lame.first * (G[0][0] + G[1][1] + G[2][2]) * glm::mat3(1.0f);
        auto P   = F * S;
        auto fs  = -glm::determinant(E) / 6.0f * P * glm::transpose(glm::inverse(E));
        auto fst = glm::transpose(fs);

        glm::vec3 f1 = fst[0];
        glm::vec3 f2 = fst[1];
        glm::vec3 f3 = fst[2];
        glm::vec3 f0 = -f1 - f2 - f3;

        _force[i0] += f0;
        _force[i1] += f1;
        _force[i2] += f2;
        _force[i3] += f3;
    }
}
