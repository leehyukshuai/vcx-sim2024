#pragma once
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"

namespace VCX::Labs::OpenProj {
    class Coord {
    public:
        Engine::GL::UniqueProgram    _program;
        Engine::GL::UniqueRenderItem _renderItem;
        Coord();

        void draw(glm::mat4 cameraTransform);
    };
} // namespace VCX::Labs::OpenProj
