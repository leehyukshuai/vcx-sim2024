#include <spdlog/spdlog.h>
#include "Engine/app.h"
#include "Labs/2-FluidSimulation/CaseFluid.h"
#include "Labs/Common/ImGuiHelper.h"
#include <iostream>
#include "Engine/loader.h"
#include "CaseFluid.h"

namespace VCX::Labs::Fluid {
    const std::vector<glm::vec3> vertex_pos = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f), 
            glm::vec3(1.0f, 1.0f, 0.0f),  
            glm::vec3(0.0f, 1.0f, 0.0f), 
            glm::vec3(0.0f, 0.0f, 1.0f),  
            glm::vec3(1.0f, 0.0f, 1.0f),   
            glm::vec3(1.0f, 1.0f, 1.0f),   
            glm::vec3(0.0f, 1.0f, 1.0f)
    };
    const std::vector<std::uint32_t> line_index = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 }; // line index

    CaseFluid::CaseFluid() :
        _program(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/fluid.vert"),
                Engine::GL::SharedShader("assets/shaders/fluid.frag") })),
        _lineprogram(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/flat.vert"),
                Engine::GL::SharedShader("assets/shaders/flat.frag") })),
        _skyboxProgram(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/skybox.vert"),
                Engine::GL::SharedShader("assets/shaders/skybox.frag") })),
        _sceneObject(1),
        _BoundaryItem(Engine::GL::VertexLayout()
            .Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream , 0), Engine::GL::PrimitiveType::Lines){ 
        _cameraManager.AutoRotate = false;
        _program.BindUniformBlock("PassConstants", 1);
        _skyboxProgram.BindUniformBlock("PassConstants", 1);
        _skyboxProgram.GetUniforms().SetByName("u_Skybox", 4);
        _lineprogram.GetUniforms().SetByName("u_Color",  glm::vec3(1.0f));
        _BoundaryItem.UpdateElementBuffer(line_index);

        _sceneObject.ReplaceScene(Engine::LoadScene("assets/scenes/fluid/myFluid.yaml"));
        _sceneObject.Skybox.value().CubeMap.SetUnit(4);
        ResetSystem();
    }

    void CaseFluid::OnSetupPropsUI() {
        if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen)) {
            if(ImGui::Button("Reset System")) {
                _stopped = true;
                ResetSystem();
            }
            ImGui::SameLine();
            if(ImGui::Button(_stopped ? "Start Simulation":"Stop Simulation"))
                _stopped = ! _stopped;
            if (_simulation.m_busyFlag) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.3f, 1.0f));
                ImGui::TextWrapped("Warning: Due to performance considerations, the simulation timestep is fixed at 0.02 seconds; try to lower Resolution to dismiss this warning");
                ImGui::PopStyleColor();
            }
            if (ImGui::SliderInt("Resolution", &_res, 4, 64))
                _recompute = true;
            ImGui::SliderFloat("Flip Ratio", &_simulation.m_fRatio, 0.0f, 1.0f);
            ImGui::Text("Mode:");
            ImGui::SameLine();
            if (ImGui::Button("Flip95")) _simulation.m_fRatio = 0.95f;
            ImGui::SameLine();
            if (ImGui::Button("Flip")) _simulation.m_fRatio = 1.0f;
            ImGui::SameLine();
            if (ImGui::Button("PIC")) _simulation.m_fRatio = 0.0f;
        }
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Appearance")) {
            _uniformDirty |= ImGui::RadioButton("Phong Model", &_useBlinn, 0);
            ImGui::SameLine();
            _uniformDirty |= ImGui::RadioButton("Blinn-Phong Model", &_useBlinn, 1);
            _uniformDirty |= ImGui::SliderFloat("Shininess", &_shininess, 1, 128, "%.1f", ImGuiSliderFlags_Logarithmic);
            _uniformDirty |= ImGui::SliderFloat("Ambient", &_ambientScale, 0.f, 2.f, "%.2fx");
            _uniformDirty |= ImGui::Checkbox("Gamma Correction", &_useGammaCorrection);
            _uniformDirty |= ImGui::SliderInt("Attenuation", &_attenuationOrder, 0, 2);

            ImGui::SliderFloat("Bnd Width", &_BndWidth, 0.f, 6.f, "%.1f");
            ImGui::Combo("Anti-Aliasing", &_msaa, "None\0002x MSAA\0004x MSAA\0008x MSAA\0");
            Common::ImGuiHelper::SaveImage(_frame.GetColorAttachment(), _frame.GetSize(), true);
        }
        ImGui::Spacing();

		if (ImGui::CollapsingHeader("Control")) {
            ImGui::Checkbox("Ease Touch", &_cameraManager.EnableDamping);
        }
        ImGui::Spacing();
    }


    Common::CaseRenderResult CaseFluid::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        if (_recompute) {
            _recompute = false;
            ResetSystem();
        }
        if (! _stopped) _simulation.SimulateTimestep(Engine::GetDeltaTime());

        OnProcessMouseControl(_cameraManager.getMouseMove());
        
        _BoundaryItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(vertex_pos));
        _frame.Resize(desiredSize, 1 << _msaa);

        _cameraManager.Update(_sceneObject.Camera);
        _sceneObject.PassConstantsBlock.Update(&VCX::Labs::Rendering::SceneObject::PassConstants::Projection, _sceneObject.Camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _sceneObject.PassConstantsBlock.Update(&VCX::Labs::Rendering::SceneObject::PassConstants::View, _sceneObject.Camera.GetViewMatrix());
        _sceneObject.PassConstantsBlock.Update(&VCX::Labs::Rendering::SceneObject::PassConstants::ViewPosition, _sceneObject.Camera.Eye);

        _lineprogram.GetUniforms().SetByName("u_Projection", _sceneObject.Camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _lineprogram.GetUniforms().SetByName("u_View"      , _sceneObject.Camera.GetViewMatrix());
        
        if (_uniformDirty) {
            _uniformDirty = false;
            _program.GetUniforms().SetByName("u_AmbientScale"      , _ambientScale);
            _program.GetUniforms().SetByName("u_UseBlinn"          , _useBlinn);
            _program.GetUniforms().SetByName("u_Shininess"         , _shininess);
            _program.GetUniforms().SetByName("u_UseGammaCorrection", int(_useGammaCorrection));
            _program.GetUniforms().SetByName("u_AttenuationOrder"  , _attenuationOrder);            
        }
        
        gl_using(_frame);

        glEnable(GL_DEPTH_TEST);
        glLineWidth(_BndWidth);
        _BoundaryItem.Draw({ _lineprogram.Use() });
        glLineWidth(1.f);
        
        auto _particleSphere = Engine::Model{.Mesh = Engine::Sphere(6,_simulation.m_particleRadius), .MaterialIndex = 0};
        Rendering::ModelObject particles = Rendering::ModelObject(_particleSphere,_simulation.m_particlePos,_simulation.m_particleColor);
        auto const & particleMaterial    = _sceneObject.Materials[0];
        particles.Mesh.Draw({ particleMaterial.Albedo.Use(),  particleMaterial.MetaSpec.Use(), particleMaterial.Height.Use(),_program.Use() },
            _particleSphere.Mesh.Indices.size(), 0, _simulation.m_iNumSpheres);

        auto _obstacleSphere = Engine::Model{.Mesh = Engine::Sphere(6,_simulation.m_obstacleRadius), .MaterialIndex = 0};
        Rendering::ModelObject obstacle = Rendering::ModelObject(_obstacleSphere,{_simulation.m_obstaclePos},{_simulation.m_obstacleColor});
        auto const & obstacleMaterial    = _sceneObject.Materials[0];
        obstacle.Mesh.Draw({ obstacleMaterial.Albedo.Use(),  obstacleMaterial.MetaSpec.Use(), obstacleMaterial.Height.Use(),_program.Use() },
            _obstacleSphere.Mesh.Indices.size(), 0, 1);

        glDepthFunc(GL_LEQUAL);
        auto const & skybox = _sceneObject.Skybox.value();
        skybox.Mesh.Draw({ skybox.CubeMap.Use(), _skyboxProgram.Use() });
        glDepthFunc(GL_LESS);
        glDisable(GL_DEPTH_TEST);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseFluid::OnProcessInput(ImVec2 const& pos) {
        _cameraManager.ProcessInput(_sceneObject.Camera, pos);
    }

    void CaseFluid::OnProcessMouseControl(glm::vec3 delta) {
        float dt = Engine::GetDeltaTime();
        float scale = 0.2f;
        auto vel = delta / dt;
        _simulation.m_obstacleVel = vel;
        _simulation.m_obstaclePos += delta * scale;
    }

    void CaseFluid::ResetSystem() {
        _simulation.initialize();
        _simulation.setupScene(_res);
    }
}