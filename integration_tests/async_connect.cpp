#include <iostream>
#include "integration_test_helper.h"
#include "global_include.h"
#include "dronecore.h"

using namespace dronecore;
using namespace std::placeholders; // for _1

static bool _discovered_device = false;
static bool _timeouted_device = false;
static uint64_t _uuid = 0;

void on_discover(uint64_t uuid);
void on_timeout(uint64_t uuid);

TEST_F(SitlTest, AsyncConnect)
{
    DroneCore dc;

    ASSERT_EQ(dc.add_udp_connection(), ConnectionResult::SUCCESS);

    dc.register_on_discover(std::bind(&on_discover, _1));
    dc.register_on_timeout(std::bind(&on_timeout, _1));

    while (!_discovered_device) {
        std::cout << "waiting for device to appear..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Let params stabilize before shutting it down.
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Call gtest to shut down SITL.
    TearDown();

    while (!_timeouted_device) {
        std::cout << "waiting for device to disappear..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void on_discover(uint64_t uuid)
{
    std::cout << "Found device with UUID: " << uuid << std::endl;
    _discovered_device = true;
    _uuid = uuid;
    // The UUID should not be 0.
    EXPECT_NE(_uuid, 0);
}

void on_timeout(uint64_t uuid)
{
    std::cout << "Lost device with UUID: " << uuid << std::endl;
    _timeouted_device = true;

    // The UUID should still be the same.
    EXPECT_EQ(_uuid, uuid);
}
