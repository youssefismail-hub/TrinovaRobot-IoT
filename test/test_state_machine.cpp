#include <unity.h>
#include "RobotStateMachine.h"

void test_initial_state_is_idle() {
    RobotStateMachine sm;
    TEST_ASSERT_TRUE(sm.current() == RobotState::Idle);
}

void test_transition_to_moving() {
    RobotStateMachine sm;
    sm.toMoving();
    TEST_ASSERT_TRUE(sm.current() == RobotState::Moving);
}

void test_emergency_stop_overrides_moving() {
    RobotStateMachine sm;
    sm.toMoving();
    sm.toEmergencyStop();
    TEST_ASSERT_TRUE(sm.current() == RobotState::EmergencyStop);
}

void test_clear_emergency_stop_returns_to_idle() {
    RobotStateMachine sm;
    sm.toEmergencyStop();
    TEST_ASSERT_TRUE(sm.clearEmergencyStop());
    TEST_ASSERT_TRUE(sm.current() == RobotState::Idle);
}

void test_clear_emergency_stop_fails_if_not_active() {
    RobotStateMachine sm;
    TEST_ASSERT_FALSE(sm.clearEmergencyStop());
}

void setup() {}
void loop() {}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_initial_state_is_idle);
    RUN_TEST(test_transition_to_moving);
    RUN_TEST(test_emergency_stop_overrides_moving);
    RUN_TEST(test_clear_emergency_stop_returns_to_idle);
    RUN_TEST(test_clear_emergency_stop_fails_if_not_active);
    UNITY_END();
    return 0;
}