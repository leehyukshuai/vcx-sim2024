#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"

#include "Object.h"

namespace VCX::Labs::OpenProj {

    class CaseNaive : public Common::ICase {
    public:
        CaseNaive();

        virtual std::string_view const GetName() override { return "Draw a 3D Box"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);

    private:
        Engine::GL::UniqueProgram     _program;
        Engine::GL::UniqueRenderFrame _frame;
        Engine::Camera                _camera { .Eye = glm::vec3(-3, 3, 3) };
        Common::OrbitCameraManager    _cameraManager;

        glm::vec3 _color {1.0, 0.6, 0.8};
        Cylinder _cylinder;
    };
} // namespace VCX::Labs::OpenProj
