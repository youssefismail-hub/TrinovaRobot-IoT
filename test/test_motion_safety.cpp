#include <unity.h>
#include "MotionSafety.h"

void test_speed_clamped_above_100() {
    RobotConfig cfg;
    MotionSafety safety(cfg);
    TEST_ASSERT_EQUAL_UINT8(100, safety.clampSpeed(150));
}

void test_speed_not_modified_in_range() {
    RobotConfig cfg;
    MotionSafety safety(cfg);
    TEST_ASSERT_EQUAL_UINT8(40, safety.clampSpeed(40));
}

void test_angle_clamped_to_config_bounds() {
    RobotConfig cfg;
    cfg.minAngle = 10;
    cfg.maxAngle = 170;
    MotionSafety safety(cfg);
    TEST_ASSERT_EQUAL_UINT8(170, safety.clampAngle(200));
    TEST_ASSERT_EQUAL_UINT8(10, safety.clampAngle(0));
}

void test_timeout_triggers_after_delay() {
    RobotConfig cfg;
    cfg.commandTimeoutMs = 100;
    MotionSafety safety(cfg);
    safety.onCommandIssued(1000);
    TEST_ASSERT_FALSE(safety.isTimedOut(1050));
    TEST_ASSERT_TRUE(safety.isTimedOut(1200));
}

void test_emergency_stop_blocks_and_clears() {
    RobotConfig cfg;
    MotionSafety safety(cfg);
    TEST_ASSERT_FALSE(safety.isEmergencyStopActive());
    safety.triggerEmergencyStop();
    TEST_ASSERT_TRUE(safety.isEmergencyStopActive());
    TEST_ASSERT_TRUE(safety.clearEmergencyStop());
    TEST_ASSERT_FALSE(safety.isEmergencyStopActive());
}

void setup() {}
void loop() {}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_speed_clamped_above_100);
    RUN_TEST(test_speed_not_modified_in_range);
    RUN_TEST(test_angle_clamped_to_config_bounds);
    RUN_TEST(test_timeout_triggers_after_delay);
    RUN_TEST(test_emergency_stop_blocks_and_clears);
    UNITY_END();
    return 0;
}