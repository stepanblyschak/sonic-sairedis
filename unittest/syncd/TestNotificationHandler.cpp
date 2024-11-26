#include "NotificationProcessor.h"
#include "NotificationHandler.h"
#include "meta/sai_serialize.h"

#include <gtest/gtest.h>

using namespace syncd;

static std::string natData =
"[{\"nat_entry\":\"{\\\"nat_data\\\":{\\\"key\\\":{\\\"dst_ip\\\":\\\"10.10.10.10\\\",\\\"l4_dst_port\\\":\\\"20006\\\",\\\"l4_src_port\\\":\\\"0\\\",\\\"proto\\\":\\\"6\\\",\\\"src_ip\\\":\\\"0.0.0.0\\\"},\\\"mask\\\":{\\\"dst_ip\\\":\\\"255.255.255.255\\\",\\\"l4_dst_port\\\":\\\"65535\\\",\\\"l4_src_port\\\":\\\"0\\\",\\\"proto\\\":\\\"255\\\",\\\"src_ip\\\":\\\"0.0.0.0\\\"}},\\\"nat_type\\\":\\\"SAI_NAT_TYPE_DESTINATION_NAT\\\",\\\"switch_id\\\":\\\"oid:0x21000000000000\\\",\\\"vr\\\":\\\"oid:0x3000000000048\\\"}\",\"nat_event\":\"SAI_NAT_EVENT_AGED\"}]";

// Test ASIC/SDK health event
std::string asheData = "{"
    "\"category\":\"SAI_SWITCH_ASIC_SDK_HEALTH_CATEGORY_FW\","
    "\"data.data_type\":\"SAI_HEALTH_DATA_TYPE_GENERAL\","
    "\"description\":\"2:30,30\","
    "\"severity\":\"SAI_SWITCH_ASIC_SDK_HEALTH_SEVERITY_FATAL\","
    "\"switch_id\":\"oid:0x21000000000000\","
    "\"timestamp\":\"{"
        "\\\"tv_nsec\\\":\\\"28715881\\\","
        "\\\"tv_sec\\\":\\\"1700042919\\\""
    "}\""
"}";


TEST(NotificationHandler, NotificationHandlerTest)
{
    std::vector<sai_attribute_t> attrs;
    sai_attribute_t attr;
    attr.id = SAI_SWITCH_ATTR_NAT_EVENT_NOTIFY;
    attr.value.ptr = (void *) 1;

    attrs.push_back(attr);

    auto notificationProcessor =
      std::make_shared<NotificationProcessor>(nullptr, nullptr, nullptr);
    auto notificationHandler =
      std::make_shared<NotificationHandler>(notificationProcessor);

    notificationHandler->updateNotificationsPointers(1, attrs.data());
    uint32_t count;
    sai_nat_event_notification_data_t *natevent = NULL;

    sai_deserialize_nat_event_ntf(natData, count, &natevent);
    notificationHandler->onNatEvent(count, natevent);

    sai_object_id_t switch_id;
    sai_switch_asic_sdk_health_severity_t severity;
    sai_timespec_t timestamp;
    sai_switch_asic_sdk_health_category_t category;
    sai_switch_health_data_t data;
    sai_u8_list_t description;
    sai_deserialize_switch_asic_sdk_health_event(asheData,
                                                 switch_id,
                                                 severity,
                                                 timestamp,
                                                 category,
                                                 data,
                                                 description);
    assert(switch_id == 0x21000000000000);
    assert(severity == SAI_SWITCH_ASIC_SDK_HEALTH_SEVERITY_FATAL);
    assert(category == SAI_SWITCH_ASIC_SDK_HEALTH_CATEGORY_FW);
    notificationHandler->onSwitchAsicSdkHealthEvent(switch_id,
                                                    severity,
                                                    timestamp,
                                                    category,
                                                    data,
                                                    description);
}

TEST(NotificationHandler, setApiVersion)
{
    auto np = std::make_shared<NotificationProcessor>(nullptr, nullptr, nullptr);

    auto nh = std::make_shared<NotificationHandler>(np);

    EXPECT_EQ(SAI_VERSION(0,0,0), nh->getApiVersion());

    nh->setApiVersion(SAI_VERSION(1,15,0));

    EXPECT_EQ(SAI_VERSION(1,15,0), nh->getApiVersion());
}
