#pragma once

#include "Engine/app.h"
#include "Labs/3-FEM/CaseStick.h"
#include "Labs/3-FEM/CaseTetra.h"
#include "Labs/Common/UI.h"
#include <vector>

namespace VCX::Labs::FEM {
    class App : public Engine::IApp {
    private:
        Common::UI _ui;

        CaseTetra _caseTetra;
        CaseStick _caseStick;

        std::size_t _caseId = 1;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = { _caseTetra, _caseStick };

    public:
        App();

        void OnFrame() override;
    };
}
