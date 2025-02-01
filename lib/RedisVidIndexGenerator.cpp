#include "RedisVidIndexGenerator.h"

#include "swss/logger.h"

#include <inttypes.h>

using namespace sairedis;

RedisVidIndexGenerator::RedisVidIndexGenerator(
        _In_ std::shared_ptr<swss::DBConnector> dbConnector,
        _In_ const std::string& vidCounterName):
    m_dbConnector(dbConnector),
    m_vidCounterName(vidCounterName)
{
    SWSS_LOG_ENTER();

    // empty
}

uint64_t RedisVidIndexGenerator::increment(uint64_t count)
{
    SWSS_LOG_ENTER();

    // this counter must be atomic since it can be independently accessed by
    // sairedis and syncd

    if (count == 1)
    {
        return m_dbConnector->incr(m_vidCounterName); // "VIDCOUNTER"
    }
    else
    {
        swss::RedisCommand sincr;
        sincr.format("INCRBY %s %" PRIu64, m_vidCounterName.c_str(), count);
        swss::RedisReply r(m_dbConnector.get(), sincr, REDIS_REPLY_INTEGER);
        return r.getContext()->integer;
    }
}

void RedisVidIndexGenerator::reset()
{
    SWSS_LOG_ENTER();

    SWSS_LOG_ERROR("not implemented");
}
