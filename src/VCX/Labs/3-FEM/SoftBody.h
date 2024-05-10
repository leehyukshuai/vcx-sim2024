#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <vector>

namespace VCX::Labs::FEM {
    class SoftBody {
        friend class Renderer;
        std::vector<glm::vec3> _velocity;
        std::vector<glm::vec3> _position;
        std::vector<glm::vec3> _rest;
        std::vector<glm::vec3> _force;
        std::vector<bool>      _fixed;

        std::vector<glm::uvec3> _faces;
        std::vector<glm::uvec4> _tetras;

        std::vector<std::function<glm::vec3(glm::vec3, glm::vec3, int)>> _constraints;

        std::pair<float, float> _lame { 5000.0f, 3000.0f }; // TODO: modify lame parameters!

    public:
        void initialize(const std::vector<glm::vec3> & position, const std::vector<glm::uvec3> & faces, const std::vector<glm::uvec4> & tetras, glm::mat4 transform);
        void initialize(const std::vector<glm::vec3> & position, const std::vector<glm::uvec3> & faces, const std::vector<glm::uvec4> & tetras, const std::vector<bool> & fixed, glm::mat4 transform);
        int  findnNearest(const glm::vec3 & eye, const glm::vec3 & dir);
        void applyConstraint(const std::function<glm::vec3(glm::vec3, glm::vec3, int)> & constraint);
        std::pair<float, float> getLame() const;
        void setLame(std::pair<float, float> lame);

        void update(float dt);

    private:
        void solveInner();
    };
} // namespace VCX::Labs::FEM