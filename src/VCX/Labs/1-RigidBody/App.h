#pragma once

#include <vector>

#include "Engine/app.h"
#include "Labs/1-RigidBody/CaseBox.h"
#include "Labs/1-RigidBody/CaseCollision.h"
#include "Labs/1-RigidBody/CaseMulti.h"
#include "Labs/Common/UI.h"

namespace VCX::Labs::RigidBody {
    class App : public Engine::IApp {
    private:
        Common::UI _ui;

        CaseBox        _caseBox;
        CaseCollision  _caseCollision;
        CaseMulti      _caseMulti;


        std::size_t _caseId = 0;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = { _caseBox, _caseCollision, _caseMulti };

    public:
        App();

        void OnFrame() override;
    };
}
