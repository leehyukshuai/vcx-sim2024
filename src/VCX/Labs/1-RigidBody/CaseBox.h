#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"
#include "RigidBody.h"
namespace VCX::Labs::RigidBody {

    class CaseBox : public Common::ICase {
    public:
        CaseBox();

        virtual std::string_view const GetName() override { return "Single Rigid Body Simulation"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);
        void OnProcessKeyControl();

    private:
        bool                                _paused { false };
        bool                                _xrayed { true };
        float                               _translationalDamping = 0.1f;
        float                               _rotationalDamping = 0.1f;

        Engine::GL::UniqueProgram           _coordProgram;
        Engine::GL::UniqueProgram           _program;
        Engine::GL::UniqueRenderFrame       _frame;
        Engine::Camera                      _camera { .Eye = glm::vec3(5, 5, 5) };
        Common::OrbitCameraManager          _cameraManager;
        Engine::GL::UniqueRenderItem        _coordItem;  // render the coordinates
        Engine::GL::UniqueIndexedRenderItem _boxItem;  // render the box
        Engine::GL::UniqueIndexedRenderItem _lineItem; // render line on box
        glm::vec3                           _dim { 1.f, 2.f, 3.f };
        glm::vec3                           _boxColor { 255.0f / 255, 150.0f / 255, 200.0f / 255 };
        RigidBody                           _rigidBody;
    };
} // namespace VCX::Labs::RigidBody
