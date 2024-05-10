#pragma once

#include "Coord.h"
#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"
#include "Renderer.h"
#include "SoftBody.h"
namespace VCX::Labs::FEM {
    class CaseStick : public Common::ICase {
    public:
        CaseStick();

        virtual std::string_view const GetName() override { return "A Elastic Stick"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);

        void initScene();

    private:
        bool _resetFlag { false };
        bool _pauseFlag { false };

        Common::OrbitCameraManager _cameraManager;
        Engine::Camera             _camera;

        Engine::GL::UniqueRenderFrame _frame;

        SoftBody _softbody;
        Renderer _renderer;
        Coord    _coord;
    };
} // namespace VCX::Labs::FEM
