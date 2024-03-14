#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"
#include "RigidBody.h"
namespace VCX::Labs::RigidBody {
    enum CollisionType {EDGE_EDGE, POINT_FACE, FACE_FACE};

    class CaseCollision : public Common::ICase {
    public:
        CaseCollision();

        virtual std::string_view const GetName() override { return "Collision between two rigid bodys"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);
        void OnProcessKeyControl();

        void ResetScene(CollisionType type);

    private:
        CollisionType                       _type { EDGE_EDGE };
        bool                                _reset { true };
        bool                                _paused { true };
        bool                                _xrayed { true };
        float                               _translationalDamping = 0.1f;
        float                               _rotationalDamping = 0.1f;

        Engine::GL::UniqueRenderFrame       _frame;
        Engine::GL::UniqueProgram           _program;
        BoxRenderItem                       _boxA;
        BoxRenderItem                       _boxB;
        Engine::GL::UniqueProgram           _coordProgram;
        Engine::GL::UniqueRenderItem        _coordItem;  // render the coordinates
        Engine::Camera                      _camera { .Eye = glm::vec3(5, 5, 5) };
        Common::OrbitCameraManager          _cameraManager;
    };
} // namespace VCX::Labs::RigidBody
