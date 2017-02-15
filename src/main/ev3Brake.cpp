#include "ev3Robot.hpp"

#ifdef BUILD_FOR_EV3

#include "ev3dev.hpp"
namespace ev3 = ev3dev;

#endif



int main(int argc, char *argv[]) {
#ifdef BUILD_FOR_EV3
    Ev3Controller::Ev3Robot robot(ev3dev::OUTPUT_A,ev3dev::OUTPUT_D,2.25);
#else
    Ev3Controller::Ev3Robot robot;
#endif

    robot.brake();
    return 0;
}
