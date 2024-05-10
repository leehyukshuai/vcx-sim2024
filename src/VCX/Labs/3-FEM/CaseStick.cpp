#include "Labs/3-FEM/CaseStick.h"
#include "Engine/app.h"
#include "Labs/Common/ImGuiHelper.h"
#include <imgui.h>

namespace VCX::Labs::FEM {
    CaseStick::CaseStick() {
        _camera.Eye = glm::vec3(3, 3, 3);

        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);

        initScene();
    }

    void CaseStick::OnSetupPropsUI() {
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

    Common::CaseRenderResult CaseStick::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // reset
        if (_resetFlag) {
            _resetFlag = false;
            initScene();
        }

        // camera control
        _cameraManager.Update(_camera);
        auto cameraTransform = _camera.GetTransformationMatrix((float(desiredSize.first) / desiredSize.second));

        // mouse control
        OnProcessMouseControl(_cameraManager.getMouseMove());

        // apply constraints
        static auto gravityFall = [=](glm::vec3 pos, glm::vec3 vel, int id) -> glm::vec3 {
            float k = 0.1f;
            return glm::vec3(0, -k, 0);
        };
        static auto floorSupport = [=](glm::vec3 pos, glm::vec3 vel, int id) -> glm::vec3 {
            float k = 1000.0f;
            if (pos.y < 0) return glm::vec3(0, -k * pos.y, 0);
            return {};
        };
        static auto translDamping = [=](glm::vec3 pos, glm::vec3 vel, int id) -> glm::vec3 {
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
            _softbody.update(0.001f);
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

    void CaseStick::OnProcessInput(ImVec2 const & pos) {
        _cameraManager.ProcessInput(_camera, pos);
    }

    void CaseStick::OnProcessMouseControl(glm::vec3 mouseDelta) {
        _softbody.applyConstraint([=](glm::vec3 pos, glm::vec3 vel, int id) -> glm::vec3 {
            return {};
        });
    }

    void CaseStick::initScene() {
        glm::uvec3 w { 1, 1, 1 };
        float delta = 1.0f;
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.1, 0.1, 0.1));


        std::vector<glm::vec3>  position;
        std::vector<bool>       fixed;
        std::vector<glm::uvec3> faces;
        std::vector<glm::uvec4> tetras;
        for (std::size_t i = 0; i <= w.x; i++) {
            for (std::size_t j = 0; j <= w.y; j++) {
                for (std::size_t k = 0; k <= w.z; k++) {
                    position.emplace_back(i * delta, j * delta, k * delta);
                    if (i == 0) {
                        fixed.push_back(true);
                    } else {
                        fixed.push_back(false);
                    }
                }
            }
        }
        auto GetID = [=](std::size_t const i, std::size_t const j, std::size_t const k) -> unsigned {
            return i * (w.y + 1) * (w.z + 1) + j * (w.z + 1) + k;
        };
        for (std::size_t i = 0; i < w.x; i++) {
            for (std::size_t j = 0; j < w.y; j++) {
                for (std::size_t k = 0; k < w.z; k++) {
                    tetras.emplace_back(GetID(i, j, k), GetID(i, j, k + 1), GetID(i, j + 1, k + 1), GetID(i + 1, j + 1, k + 1));
                    tetras.emplace_back(GetID(i, j, k), GetID(i, j + 1, k), GetID(i, j + 1, k + 1), GetID(i + 1, j + 1, k + 1));
                    tetras.emplace_back(GetID(i, j, k), GetID(i, j, k + 1), GetID(i + 1, j, k + 1), GetID(i + 1, j + 1, k + 1));
                    tetras.emplace_back(GetID(i, j, k), GetID(i + 1, j, k), GetID(i + 1, j, k + 1), GetID(i + 1, j + 1, k + 1));
                    tetras.emplace_back(GetID(i, j, k), GetID(i, j + 1, k), GetID(i + 1, j + 1, k), GetID(i + 1, j + 1, k + 1));
                    tetras.emplace_back(GetID(i, j, k), GetID(i + 1, j, k), GetID(i + 1, j + 1, k), GetID(i + 1, j + 1, k + 1));
                }
            }
        }
        for (std::size_t j = 0; j < w.y; j++) {
            for (std::size_t k = 0; k < w.z; k++) {
                faces.emplace_back(GetID(0, j, k), GetID(0, j + 1, k), GetID(0, j + 1, k + 1));
                faces.emplace_back(GetID(0, j, k), GetID(0, j, k + 1), GetID(0, j + 1, k + 1));
                faces.emplace_back(GetID(w.x, j, k), GetID(w.x, j + 1, k), GetID(w.x, j + 1, k + 1));
                faces.emplace_back(GetID(w.x, j, k), GetID(w.x, j, k + 1), GetID(w.x, j + 1, k + 1));
            }
        }
        for (std::size_t i = 0; i < w.x; i++) {
            for (std::size_t k = 0; k < w.z; k++) {
                faces.emplace_back(GetID(i, 0, k), GetID(i + 1, 0, k), GetID(i + 1, 0, k + 1));
                faces.emplace_back(GetID(i, 0, k), GetID(i, 0, k + 1), GetID(i + 1, 0, k + 1));
                faces.emplace_back(GetID(i, w.y, k), GetID(i + 1, w.y, k), GetID(i + 1, w.y, k + 1));
                faces.emplace_back(GetID(i, w.y, k), GetID(i, w.y, k + 1), GetID(i + 1, w.y, k + 1));
            }
        }
        for (std::size_t i = 0; i < w.x; i++) {
            for (std::size_t j = 0; j < w.y; j++) {
                faces.emplace_back(GetID(i, j, 0), GetID(i + 1, j, 0), GetID(i, j + 1, 0));
                faces.emplace_back(GetID(i + 1, j + 1, 0), GetID(i + 1, j, 0), GetID(i, j + 1, 0));
                faces.emplace_back(GetID(i, j, w.z), GetID(i + 1, j, w.z), GetID(i + 1, j + 1, w.z));
                faces.emplace_back(GetID(i, j, w.z), GetID(i, j + 1, w.z), GetID(i + 1, j + 1, w.z));
            }
        }

        _softbody.initialize(position, faces, tetras, fixed, transform);
        _renderer.bind(_softbody);
    }
} // namespace VCX::Labs::FEM
