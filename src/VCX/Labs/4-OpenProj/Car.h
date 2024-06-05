#pragma once

#include "Object.h"

namespace VCX::Labs::OpenProj {
    class Car {
    private:
        glm::vec3 calcCenter();
    public:
        Box                   body;
        std::vector<Sphere>   wheels;
        std::vector<glm::vec3> relatives;
        std::vector<Object *> objects();

        Car();
        void reset();
        void rigidify();
        // keyMove: WASD
        void move(bool keyMove[4]);
    };

    class Road {
    public:
        std::vector<Box>      sects;
        std::vector<Object *> objects();

        Road();
        void reset();
    };

} // namespace VCX::Labs::OpenProj
