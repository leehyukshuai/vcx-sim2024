#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"

#include "Object.h"
#include "Coord.h"


namespace VCX::Labs::OpenProj {

    class CaseNaive : public Common::ICase {
    public:
        CaseNaive();

        virtual std::string_view const GetName() override { return "Naive"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);

        void resetScene();

    private:
        Engine::GL::UniqueRenderFrame _frame;
        Engine::Camera                _camera { .Eye = glm::vec3(-6, 6, 6) };
        Common::OrbitCameraManager    _cameraManager;

        bool _pause { true };

        float _gravity              = 2.0f;
        float _translationalDamping = 0.1f;
        float _rotationalDamping    = 0.02f;

        CollisionSystem _collisionSystem;
        RenderSystem    _renderSystem;
        Coord _coord;

        Sphere *_sphere;
        Box    *_floor;
    };
} // namespace VCX::Labs::OpenProj
