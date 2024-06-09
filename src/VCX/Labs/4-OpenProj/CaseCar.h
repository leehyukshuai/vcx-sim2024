#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"

#include "Car.h"
#include "Collision.h"
#include "Coord.h"
#include "Object.h"

namespace VCX::Labs::OpenProj {
    class CaseCar : public Common::ICase {
    public:
        CaseCar();

        virtual std::string_view const GetName() override { return "Prototype Car"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);
        void OnProcessKeyControl();

        void resetScene();

    private:
        Engine::GL::UniqueRenderFrame _frame;
        Engine::Camera                _camera { .Eye = glm::vec3(-30, 20, 20) };
        Common::OrbitCameraManager    _cameraManager;

        bool _pause { false };

        float _gravity              = 2.0f;
        float _translationalDamping = 0.1f;
        float _rotationalDamping    = 0.02f;

        bool keyMove[5] { false };

        CollisionSystem _collisionSystem;
        RenderSystem    _renderSystem;

        Car   _car;
        Road  _road;
        Coord _coord;
    };
} // namespace VCX::Labs::OpenProj
