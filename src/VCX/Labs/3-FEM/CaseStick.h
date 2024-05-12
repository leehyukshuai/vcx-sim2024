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
#include <unordered_set>
namespace VCX::Labs::FEM {
    class CaseStick : public Common::ICase {
    public:
        CaseStick();

        virtual std::string_view const GetName() override { return "An Elastic Stick"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);

        void initScene(glm::uvec3 w);

    private:
        float _gravity { 10.0f };
        bool _showCoord { false };
        bool _cameraFlag { false };
        bool _resetFlag { false };
        bool _pauseFlag { false };

        float _youngModulus { 10000.0f };
        float _poissonRatio { 0.3f };

        glm::uvec3 _w { 16, 5, 5 };

        std::unordered_set<unsigned> _fixed;

        Common::OrbitCameraManager _cameraManager;
        Engine::Camera             _camera;

        Engine::GL::UniqueRenderFrame _frame;

        SoftBody _softbody;
        Renderer _renderer;
        Coord    _coord;
    };
} // namespace VCX::Labs::FEM
