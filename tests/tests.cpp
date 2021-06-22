#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "lib/inc/ZeroMQChannel.h"
#include "lib/inc/Recorder.h"
#include "syncd/ZeroMQNotificationProducer.h"

#include "meta/sai_serialize.h"

#include <thread>
#include <memory>

using namespace sairedis;
using namespace syncd;

#define ASSERT_EQ(a,b) if ((a) != (b)) { SWSS_LOG_THROW("ASSERT EQ FAILED: " #a " != " #b); }

const std::string SairedisRecFilename = "sairedis.rec";

/*
 * Test if destructor proper clean and join zeromq socket and context, and
 * break recv method.
 */
static void test_zeromqchannel_destructor()
{
    SWSS_LOG_ENTER();

    std::cout << " * " << __FUNCTION__ << std::endl;

    for (int i = 0; i < 10; i++)
    {
        ZeroMQChannel z("ipc:///tmp/feeds1", "ipc:///tmp/feeds2", nullptr);

        usleep(10*1000);
    }
}

/*
 * Test if first notification sent from notification producer will arrive at
 * zeromq channel notification thread. There is an issue with PUB/SUB inside
 * zeromq, and in our model this was changed to push/pull model.
 */
static void test_zeromqchannel_first_notification()
{
    SWSS_LOG_ENTER();

    std::cout << " * " << __FUNCTION__ << std::endl;

    for (int i = 0; i < 10; i++)
    {
        std::string rop;
        std::string rdata;

        bool got = false;

        auto callback = [&](
                const std::string& op,
                const std::string& data,
                const std::vector<swss::FieldValueTuple>& values)
        {
            SWSS_LOG_NOTICE("got: %s %s", op.c_str(), data.c_str());

            rop = op;
            rdata = data;

            got = true;
        };

        ZeroMQChannel z("ipc:///tmp/feeds1", "ipc:///tmp/feeds2", callback);

        ZeroMQNotificationProducer p("ipc:///tmp/feeds2");

        p.send("foo", "bar", {});

        int count = 0;

        while (!got && count++ < 200) // in total we will wait max 2 seconds
        {
            usleep(10*1000);
        }

        ASSERT_EQ(rop, "foo");
        ASSERT_EQ(rdata, "bar");
    }
}

std::vector<std::string> parseFirstRecordedAPI()
{
    const auto delimiter = '|';
    std::ifstream infile(SairedisRecFilename);
    std::string line;
    // skip first line
    std::getline(infile, line);
    std::getline(infile, line);

    std::vector<std::string> tokens;
    std::stringstream sstream(line);
    std::string token;
    // skip first, it is a timestamp
    std::getline(sstream, token, delimiter);
    while(std::getline(sstream, token, delimiter)) {
       tokens.push_back(token);
    }
    return tokens;
}

void test_recorder_enum_value_capability_query_request(
    sai_object_id_t switch_id,
    sai_object_type_t object_type,
    sai_attr_id_t attr_id,
    const std::vector<std::string>& expectedOutput)
{
    SWSS_LOG_ENTER();

    std::cout << " * " << __FUNCTION__ << std::endl;

    remove(SairedisRecFilename.c_str());

    Recorder recorder;
    recorder.enableRecording(true);

    sai_s32_list_t enum_values_capability {.count = 0, .list = nullptr};

    recorder.recordQueryAattributeEnumValuesCapability(
        switch_id,
        object_type,
        attr_id,
        &enum_values_capability
    );

    auto tokens = parseFirstRecordedAPI();
    ASSERT_EQ(tokens, expectedOutput);
}

void test_recorder_enum_value_capability_query_response(
    sai_status_t status,
    sai_object_type_t object_type,
    sai_attr_id_t attr_id,
    std::vector<int32_t> enumList,
    const std::vector<std::string>& expectedOutput)
{
    SWSS_LOG_ENTER();

    std::cout << " * " << __FUNCTION__ << std::endl;

    remove(SairedisRecFilename.c_str());

    Recorder recorder;
    recorder.enableRecording(true);

    sai_s32_list_t enum_values_capability;
    enum_values_capability.count = static_cast<int32_t>(enumList.size());
    enum_values_capability.list = enumList.data();

    recorder.recordQueryAattributeEnumValuesCapabilityResponse(
        status,
        object_type,
        attr_id,
        &enum_values_capability
    );

    auto tokens = parseFirstRecordedAPI();
    ASSERT_EQ(tokens, expectedOutput);
}

void test_recorder_enum_value_capability_query()
{
    SWSS_LOG_ENTER();

    test_recorder_enum_value_capability_query_request(
        1,
        SAI_OBJECT_TYPE_DEBUG_COUNTER,
        SAI_DEBUG_COUNTER_ATTR_TYPE,
        {
            "q",
            "attribute_enum_values_capability",
            "SAI_OBJECT_TYPE_SWITCH:oid:0x1",
            "SAI_DEBUG_COUNTER_ATTR_TYPE=0",
        }
    );
    test_recorder_enum_value_capability_query_response(
        SAI_STATUS_SUCCESS,
        SAI_OBJECT_TYPE_DEBUG_COUNTER,
        SAI_DEBUG_COUNTER_ATTR_TYPE,
        {
            SAI_DEBUG_COUNTER_TYPE_PORT_IN_DROP_REASONS,
            SAI_DEBUG_COUNTER_TYPE_PORT_OUT_DROP_REASONS,
            SAI_DEBUG_COUNTER_TYPE_SWITCH_IN_DROP_REASONS,
            SAI_DEBUG_COUNTER_TYPE_SWITCH_OUT_DROP_REASONS,
        },
        {
            "Q",
            "attribute_enum_values_capability",
            "SAI_STATUS_SUCCESS",
            "SAI_DEBUG_COUNTER_ATTR_TYPE=4:SAI_DEBUG_COUNTER_TYPE_PORT_IN_DROP_REASONS,SAI_DEBUG_COUNTER_TYPE_PORT_OUT_DROP_REASONS,"
            "SAI_DEBUG_COUNTER_TYPE_SWITCH_IN_DROP_REASONS,SAI_DEBUG_COUNTER_TYPE_SWITCH_OUT_DROP_REASONS",
        }
    );
    test_recorder_enum_value_capability_query_request(
        1,
        SAI_OBJECT_TYPE_DEBUG_COUNTER,
        SAI_DEBUG_COUNTER_ATTR_IN_DROP_REASON_LIST,
        {
            "q",
            "attribute_enum_values_capability",
            "SAI_OBJECT_TYPE_SWITCH:oid:0x1",
            "SAI_DEBUG_COUNTER_ATTR_IN_DROP_REASON_LIST=0",
        }
    );
    test_recorder_enum_value_capability_query_response(
        SAI_STATUS_SUCCESS,
        SAI_OBJECT_TYPE_DEBUG_COUNTER,
        SAI_DEBUG_COUNTER_ATTR_IN_DROP_REASON_LIST,
        {
            SAI_IN_DROP_REASON_L2_ANY,
            SAI_IN_DROP_REASON_L3_ANY
        },
        {
            "Q",
            "attribute_enum_values_capability",
            "SAI_STATUS_SUCCESS",
            "SAI_DEBUG_COUNTER_ATTR_IN_DROP_REASON_LIST=2:SAI_IN_DROP_REASON_L2_ANY,SAI_IN_DROP_REASON_L3_ANY"
        }
    );

}

int main()
{
    SWSS_LOG_ENTER();

    test_zeromqchannel_destructor();

    test_zeromqchannel_first_notification();

    test_recorder_enum_value_capability_query();

    return 0;
}
