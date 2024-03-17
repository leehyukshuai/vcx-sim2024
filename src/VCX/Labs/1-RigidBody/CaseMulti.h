#pragma once

#include "Collision.h"
#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"
#include "RigidBody.h"
namespace VCX::Labs::RigidBody {
    class CaseMulti : public Common::ICase {
    public:
        CaseMulti();

        virtual std::string_view const GetName() override { return "Collision between multiple rigid bodys"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);
        void OnProcessKeyControl();

        void ResetScene();

    private:
        bool      _reset { true };
        bool      _paused { true };
        float     _translationalDamping = 0.1f;
        float     _rotationalDamping    = 0.1f;
        glm::vec3 _gravity              = glm::vec3(0, -1, 0);

        Engine::GL::UniqueRenderFrame _frame;
        Engine::GL::UniqueProgram     _program;
        Engine::GL::UniqueProgram     _coordProgram;
        Engine::GL::UniqueRenderItem  _coordItem; // render the coordinates
        Engine::Camera                _camera { .Eye = glm::vec3(5, 5, 5) };
        Common::OrbitCameraManager    _cameraManager;
        std::vector<BoxRenderItem>    _boxes;
        BoxCollisionSystem            _collisionSystem;
    };
} // namespace VCX::Labs::RigidBody
