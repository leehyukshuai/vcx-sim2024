#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"

#include "Collision.h"
#include "Object.h"
#include "Coord.h"

namespace VCX::Labs::OpenProj {
    class Car {
    public:
        Box                 body;
        std::vector<Sphere> wheels;

        Car() {
            reset();
        }

        std::vector<Object *> objects() {
            return std::vector<Object *>({ &body, &wheels[0], &wheels[1], &wheels[2], &wheels[3] });
        }
        void handleUserInput() {
            float rs = 10.0f;
            if (ImGui::IsKeyDown(ImGuiKey_J)) {
                for (auto & w : wheels) w.rigidBody->applyTorque({ -rs, 0, 0 });
            }
            if (ImGui::IsKeyDown(ImGuiKey_L)) {
                for (auto & w : wheels) w.rigidBody->applyTorque({ rs, 0, 0 });
            }
            if (ImGui::IsKeyDown(ImGuiKey_I)) {
                for (auto & w : wheels) w.rigidBody->applyTorque({ 0, 0, -rs });
            }
            if (ImGui::IsKeyDown(ImGuiKey_K)) {
                for (auto & w : wheels) w.rigidBody->applyTorque({ 0, 0, rs });
            }
        }
        void rigidify() {
            // TODO: shape match
            return;
        }
        void reset() {
            body.boxBody.reset();
            body.boxBody.dimension = glm::vec3(8, 3, 5);
            body.boxBody.position  = glm::vec3(0, 3.7, 0);
            body.renderItem.color = glm::vec3(0.7f, 0.3f, 0.9f);
            body.initialize();

            wheels.clear();
            wheels.resize(4);
            wheels[0].sphereBody.position = glm::vec3(-3, 1.1, -1.5);
            wheels[1].sphereBody.position = glm::vec3(3, 1.1, 1.5);
            wheels[2].sphereBody.position = glm::vec3(-3, 1.1, 1.5);
            wheels[3].sphereBody.position = glm::vec3(3, 1.1, -1.5);
            for (auto & w : wheels) {
                w.sphereBody.radius = 1.0f;
                w.renderItem.color  = glm::vec3(0.2f, 0.2f, 0.3f);
                w.initialize();
            }
        }
    };

    class Road {
    public:
        std::vector<Box> sects;

        Road() {
            reset();
        }

        std::vector<Object *> objects() {
            std::vector<Object *> ret;
            for (auto & s : sects) {
                ret.push_back(&s);
            }
            return ret;
        }
        void reset() {
            sects.clear();
            sects.resize(3);
            sects[0].boxBody.position    = glm::vec3(-29.86, 7.5, 0);
            sects[0].boxBody.orientation = glm::quat(glm::vec3(0, 0, -glm::radians(30.0f)));
            sects[1].boxBody.position    = glm::vec3(0, -0.5, 0);
            sects[1].boxBody.orientation = glm::quat(glm::vec3(0, 0, 0));
            sects[2].boxBody.position    = glm::vec3(29.86, 7.5, 0);
            sects[2].boxBody.orientation = glm::quat(glm::vec3(0, 0, glm::radians(30.0f)));
            for (auto & w : sects) {
                w.boxBody.dimension = glm::vec3(32, 1, 64);
                w.renderItem.color  = glm::vec3(0.2f, 1.0f, 0.3f);
                w.initialize();
            }
        }
    };

    class CaseCar : public Common::ICase {
    public:
        CaseCar();

        virtual std::string_view const GetName() override { return "Collision"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

        void OnProcessMouseControl(glm::vec3 mourseDelta);

        void resetScene();

    private:
        Engine::GL::UniqueProgram     _program;
        Engine::GL::UniqueRenderFrame _frame;
        Engine::Camera                _camera { .Eye = glm::vec3(-10, 5, 5), .Target = glm::vec3(0, 2, 0) };
        Common::OrbitCameraManager    _cameraManager;

        bool _pause { true };

        float _gravity              = 2.0f;
        float _translationalDamping = 0.1f;
        float _rotationalDamping    = 0.02f;

        CollisionSystem _collisionSystem;
        RenderSystem    _renderSystem;

        Car  _car;
        Road _road;
        Coord _coord;
    };
} // namespace VCX::Labs::OpenProj
