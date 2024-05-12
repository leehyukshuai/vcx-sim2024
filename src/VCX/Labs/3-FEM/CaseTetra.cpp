#include "Labs/3-FEM/CaseTetra.h"
#include "Engine/app.h"
#include "Labs/Common/ImGuiHelper.h"
#include <imgui.h>

namespace VCX::Labs::FEM {
    CaseTetra::CaseTetra() {
        _camera.Eye = glm::vec3(3, 3, 3);

        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);

        initScene();
    }

    void CaseTetra::OnSetupPropsUI() {
        if (ImGui::Button("Reset")) {
            _resetFlag = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(_pauseFlag ? "Start" : "Pause")) {
            _pauseFlag = ! _pauseFlag;
        }
        if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto lame = _softbody.getLame();
            ImGui::DragFloat("lame lambda", &lame.first);
            ImGui::DragFloat("lame miu", &lame.second);
            _softbody.setLame(lame);
        }
    }

    Common::CaseRenderResult CaseTetra::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // reset
        if (_resetFlag) {
            _resetFlag = false;
            _cameraManager.Reset(_camera);
            initScene();
        }

        // camera control
        _cameraManager.Update(_camera);
        auto cameraTransform = _camera.GetTransformationMatrix((float(desiredSize.first) / desiredSize.second));

        // mouse control
        OnProcessMouseControl(_cameraManager.getMouseMove());

        // apply constraints
        static auto gravityFall = [=](glm::vec3 & pos, glm::vec3 & vel, int id) -> glm::vec3 {
            float k = 0.1f;
            return glm::vec3(0, -k, 0);
        };
        static auto floorSupport = [=](glm::vec3 & pos, glm::vec3 & vel, int id) -> glm::vec3 {
            float k = 1000.0f;
            if (pos.y < 0) return glm::vec3(0, -k * pos.y, 0);
            return {};
        };
        static auto translDamping = [=](glm::vec3 & pos, glm::vec3 & vel, int id) -> glm::vec3 {
            float k = 2.0f;
            auto  n = glm::normalize(vel);
            if (! std::isnan(n.x)) return -k * vel * vel * n;
            return {};
        };
        _softbody.applyConstraint(gravityFall);
        _softbody.applyConstraint(floorSupport);
        _softbody.applyConstraint(translDamping);

        // update model
        if (! _pauseFlag) {
            _softbody.update(0.01f);
        } else {
            _softbody.update(0.0f);
        }
        _renderer.update(_softbody);

        // begin rendering
        _frame.Resize(desiredSize);
        gl_using(_frame);
        _renderer.draw(cameraTransform, { 0.5f, 0.5f, 1.0f }, { 0.5f, 1.0f, 0.5f });
        _coord.draw(cameraTransform);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseTetra::OnProcessInput(ImVec2 const & pos) {
        _cameraManager.ProcessInput(_camera, pos);
    }

    void CaseTetra::OnProcessMouseControl(glm::vec3 mouseDelta) {
        _softbody.applyConstraint([=](glm::vec3 pos, glm::vec3 vel, int id) -> glm::vec3 {
            float k = 2.0f;
            if (id == 0)
                return k * mouseDelta;
            return {};
        });
    }

    void CaseTetra::initScene() {
        std::vector<glm::vec3> position {
            { -1, 0, -1 },
            {  1, 0,  0 },
            {  0, 1,  0 },
            {  0, 0,  1 }
        };
        const std::vector<glm::uvec3> faces {
            { 0, 1, 2 },
            { 0, 1, 3 },
            { 0, 2, 3 },
            { 1, 2, 3 }
        };
        const std::vector<glm::uvec4> tetras {
            { 0, 1, 2, 3 }
        };
        glm::mat4 transform = glm::mat4(1.0f);
        _softbody.initialize(position, faces, tetras, transform);
        _renderer.bind(_softbody);
    }
} // namespace VCX::Labs::FEM
