#pragma once

#include "CaseCollide.h"
#include "CaseNaive.h"
#include "CaseCar.h"
#include "Engine/app.h"
#include "Labs/Common/UI.h"
#include <vector>

namespace VCX::Labs::OpenProj {
    class App : public Engine::IApp {
    private:
        Common::UI _ui;

        CaseNaive   _caseNaive;
        CaseCollide _caseCollide;
        CaseCar _caseCar;

        std::size_t _caseId = 2;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = { _caseNaive, _caseCollide, _caseCar };

    public:
        App();

        void OnFrame() override;
    };
}
