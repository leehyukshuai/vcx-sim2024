#include "Assets/bundled.h"
#include "Labs/2-FluidSimulation/App.h"

int main() {
    using namespace VCX;
    return Engine::RunApp<Labs::Fluid::App>(Engine::AppContextOptions {
        .Title         = "VCX-sim Labs 2: Fluid Simulation",
        .WindowSize    = {1024, 768},
        .FontSize      = 16,
        .IconFileNames = Assets::DefaultIcons,
        .FontFileNames = Assets::DefaultFonts,
    });
}
