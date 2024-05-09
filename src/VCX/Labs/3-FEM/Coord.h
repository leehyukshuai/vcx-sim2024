#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"

namespace VCX::Labs::FEM {
    class Coord {
    public:
        Engine::GL::UniqueProgram _program;
        Engine::GL::UniqueRenderItem  _renderItem;
        Coord();

        void draw(glm::mat4 cameraTransform);
    };
} // namespace FEM
