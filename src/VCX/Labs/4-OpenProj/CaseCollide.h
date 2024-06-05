#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"

#include "Collision.h"
#include "Object.h"

namespace VCX::Labs::OpenProj {

    class CaseCollide : public Common::ICase {
    public:
        CaseCollide();

        virtual std::string_view const GetName() override { return "Collision"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);

        void resetScene();

    private:
        Engine::GL::UniqueProgram     _program;
        Engine::GL::UniqueRenderFrame _frame;
        Engine::Camera                _camera { .Eye = glm::vec3(-3, 3, 3) };
        Common::OrbitCameraManager    _cameraManager;

        glm::vec3 _gravity = glm::vec3(0, 2, 0);

        CollisionSystem       _collisionSystem;
        std::vector<Cylinder> _cylinders;
        Box                   _floor;
    };
} // namespace VCX::Labs::OpenProj
