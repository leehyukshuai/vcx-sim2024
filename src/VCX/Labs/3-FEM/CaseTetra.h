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

    class CaseTetra : public Common::ICase {
    public:
        CaseTetra();

        virtual std::string_view const GetName() override { return "Single Tetrahedron Demo"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);

        void initScene(int resolution);

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
