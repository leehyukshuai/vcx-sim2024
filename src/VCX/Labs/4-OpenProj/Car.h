#pragma once

#include "Object.h"

namespace VCX::Labs::OpenProj {
    class Car {
    private:
        std::vector<std::vector<float>> distMatrix;

    public:
        float stiffness { 128.0f };
        float torque { 10.0f };
        float brake { 16.0f };

        Box                 body;
        std::vector<Sphere> wheels;

        std::vector<Object *> objects();

        Car();
        void reset();
        void rigidify();
        // keyMove: W+A+S+D+Brake
        void move(bool keyMove[5]);
    };

    class Road {
    public:
        std::vector<Box>      sects;
        std::vector<Object *> objects();

        Road();
        void reset();
    };

} // namespace VCX::Labs::OpenProj
