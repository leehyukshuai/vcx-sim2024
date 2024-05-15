#pragma once
#include "SoftBody.h"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"

namespace VCX::Labs::FEM {
    class Renderer {
        bool _xRay { true };
        Engine::GL::UniqueProgram     _program;
        Engine::GL::UniqueIndexedRenderItem _faceItem;
        Engine::GL::UniqueIndexedRenderItem _lineItem;
    public:
        Renderer();
        void setXRay(bool xRay);
        void bind(const SoftBody &softBody);
        void update(const SoftBody &softBody);
        void draw(glm::mat4 cameraTransform, glm::vec3 faceColor, glm::vec3 lineColor);
    };
} // namespace VCX::Labs::FEM