#include "Labs/1-RigidBody/CaseBox.h"
#include "Labs/Common/ImGuiHelper.h"
#include "Engine/app.h"
#include "CaseBox.h"
#include <imgui.h>


namespace VCX::Labs::RigidBody {
    static constexpr auto c_PositionData = std::to_array<glm::vec3>({
        {-1000, 0, 0},
        {1000, 0, 0},
        {0, -1000, 0},
        {0, 1000, 0},
        {0, 0, -1000},
        {0, 0, 1000},
    });

    static constexpr auto c_ColorData = std::to_array<glm::vec3>({
        {1, 0, 0},
        {1, 0, 0},
        {0, 1, 0},
        {0, 1, 0},
        {0, 0, 1},
        {0, 0, 1},
    });


    CaseBox::CaseBox():
        _program(
            Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat.vert"),
                                        Engine::GL::SharedShader("assets/shaders/flat.frag") })),
        _coordProgram(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/coord.vert"),
                                        Engine::GL::SharedShader("assets/shaders/coord.frag") })),
        _boxItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Triangles),
        _lineItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Lines),
        _coordItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Static, 0)
            .Add<glm::vec3>("color", Engine::GL::DrawFrequency::Static, 1), Engine::GL::PrimitiveType::Lines) {
        //     3-----2
        //    /|    /|
        //   0 --- 1 |
        //   | 7 - | 6
        //   |/    |/
        //   4 --- 5
        const std::vector<std::uint32_t> line_index = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 }; // line index
        _lineItem.UpdateElementBuffer(line_index);

        const std::vector<std::uint32_t> tri_index = { 0, 1, 2, 0, 2, 3, 1, 4, 0, 1, 4, 5, 1, 6, 5, 1, 2, 6, 2, 3, 7, 2, 6, 7, 0, 3, 7, 0, 4, 7, 4, 5, 6, 4, 6, 7 };
        _boxItem.UpdateElementBuffer(tri_index);

        _coordItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(c_PositionData));
        _coordItem.UpdateVertexBuffer("color", Engine::make_span_bytes<glm::vec3>(c_ColorData));

        _cameraManager.AutoRotate = false;
        _cameraManager.Save(_camera);
    }

    void CaseBox::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button(!_paused ? "pause" : "start")) {
                _paused = !_paused;
            }
        }
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("z-view")) {
                _cameraManager.Save(Engine::Camera({.Eye = glm::vec3(0,0,10)}));
                _cameraManager.Reset(_camera);
            }
            ImGui::SameLine();
            if (ImGui::Button("x-view")) {
                _cameraManager.Save(Engine::Camera({.Eye = glm::vec3(10,0,0)}));
                _cameraManager.Reset(_camera);
            }
            ImGui::SameLine();
            if (ImGui::Button("y-view")) {
                _cameraManager.Save(Engine::Camera({.Eye = glm::vec3(0,10,0)}));
                _cameraManager.Reset(_camera);
            }
        }
        if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat("transl damping", &_translationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("rotate damping", &_rotationalDamping, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::InputFloat3("position", glm::value_ptr(_rigidBody.position), "%.1f");
            ImGui::InputFloat3("velocity", glm::value_ptr(_rigidBody.velocity), "%.1f");
            auto eulerAngles = glm::eulerAngles(_rigidBody.orientation) * 180.0f / glm::pi<float>();
            ImGui::InputFloat3("orientation", glm::value_ptr(eulerAngles), "%.1f");
            eulerAngles *= glm::pi<float>() / 180.0f;
            _rigidBody.orientation = glm::quat(eulerAngles);
            ImGui::InputFloat3("omega", glm::value_ptr(_rigidBody.omega), "%.1f");
            ImGui::SliderFloat("dim-x", &_dim[0], 0.5, 4);
            ImGui::SliderFloat("dim-y", &_dim[1], 0.5, 4);
            ImGui::SliderFloat("dim-z", &_dim[2], 0.5, 4);
        }
        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::ColorEdit3("Box Color", glm::value_ptr(_boxColor));
        }
    }

    Common::CaseRenderResult CaseBox::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        // apply control first
        OnProcessMouseControl(_cameraManager.getMouseMove());
        OnProcessKeyControl();

        std::vector<glm::vec3> VertsPosition;
        glm::vec3              new_x = _dim[0] / 2 * glm::vec3(1.f, 0.f, 0.f);
        glm::vec3              new_y = _dim[1] / 2 * glm::vec3(0.f, 1.f, 0.f);
        glm::vec3              new_z = _dim[2] / 2 * glm::vec3(0.f, 0.f, 1.f);
        VertsPosition.resize(8);
        VertsPosition[0] = - new_x + new_y + new_z;
        VertsPosition[1] = + new_x + new_y + new_z;
        VertsPosition[2] = + new_x + new_y - new_z;
        VertsPosition[3] = - new_x + new_y - new_z;
        VertsPosition[4] = - new_x - new_y + new_z;
        VertsPosition[5] = + new_x - new_y + new_z;
        VertsPosition[6] = + new_x - new_y - new_z;
        VertsPosition[7] = - new_x - new_y - new_z;
        // update mass
        _rigidBody.mass = (_dim[0] * _dim[1] * _dim[2]) / 6.0f;
        // update inertia
        _rigidBody.inertia[0][0] = _rigidBody.mass/12.0*(_dim[1]*_dim[1]+_dim[2]*_dim[2]);
        _rigidBody.inertia[1][1] = _rigidBody.mass/12.0*(_dim[0]*_dim[0]+_dim[2]*_dim[2]);
        _rigidBody.inertia[2][2] = _rigidBody.mass/12.0*(_dim[0]*_dim[0]+_dim[1]*_dim[1]);
        
        // apply damping

        if (_rigidBody.velocity != glm::vec3(0,0,0))
            _rigidBody.apply(glm::normalize(_rigidBody.velocity) * glm::length2(_rigidBody.velocity) * -_translationalDamping);
        if (_rigidBody.omega != glm::vec3(0,0,0))
            _rigidBody.applyTorque(glm::normalize(_rigidBody.omega) * glm::length2(_rigidBody.omega) * -_rotationalDamping);

        // update
        if (_paused) {
            _rigidBody.resetForces();
        } else {
            _rigidBody.update(Engine::GetDeltaTime());
        }

        // rendering
        _frame.Resize(desiredSize);

        _cameraManager.Update(_camera);
        _program.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _program.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());
        _coordProgram.GetUniforms().SetByName("u_Projection", _camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _coordProgram.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());


        gl_using(_frame);
        // glEnable(GL_LINE_SMOOTH);
        // glLineWidth(.5f);

        for (auto &vert : VertsPosition) {
            // rotational state
            vert = glm::rotate(_rigidBody.orientation, vert);
            // translational state
            vert += _rigidBody.position;
        }

        auto span_bytes = Engine::make_span_bytes<glm::vec3>(VertsPosition);

        _program.GetUniforms().SetByName("u_Color", _boxColor);
        _boxItem.UpdateVertexBuffer("position", span_bytes);
        _boxItem.Draw({ _program.Use() });

        _program.GetUniforms().SetByName("u_Color", glm::vec3(1.f, 1.f, 1.f));
        _lineItem.UpdateVertexBuffer("position", span_bytes);
        _lineItem.Draw({ _program.Use() });

        _coordItem.Draw({ _coordProgram.Use() });

        // glLineWidth(1.f);
        // glDisable(GL_LINE_SMOOTH);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseBox::OnProcessInput(ImVec2 const & pos) {
        _cameraManager.ProcessInput(_camera, pos);
    }

    void CaseBox::OnProcessMouseControl(glm::vec3 mouseDelta) {
        // float movingScale = 0.1f;
    }

    void CaseBox::OnProcessKeyControl() {
        if (ImGui::IsKeyDown(ImGuiKey_J)) {
            _rigidBody.apply({-1, 0, 0});
        }
        if (ImGui::IsKeyDown(ImGuiKey_L)) {
            _rigidBody.apply({1, 0, 0});
        }
        if (ImGui::IsKeyDown(ImGuiKey_I)) {
            _rigidBody.apply({0, 0, -1});
        }
        if (ImGui::IsKeyDown(ImGuiKey_K)) {
            _rigidBody.apply({0, 0, 1});
        }
        if (ImGui::IsKeyDown(ImGuiKey_U)) {
            _rigidBody.applyTorque({10, 0, 0});
        }
        if (ImGui::IsKeyDown(ImGuiKey_O)) {
            _rigidBody.applyTorque({0, -10, 0});
        }
    }

} // namespace VCX::Labs::RigidBody
