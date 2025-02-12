#include "NumberOidIndexGenerator.h"

#include "swss/logger.h"

using namespace saimeta;

NumberOidIndexGenerator::NumberOidIndexGenerator()
{
    SWSS_LOG_ENTER();

    reset();
}

uint64_t NumberOidIndexGenerator::increment(uint64_t count)
{
    SWSS_LOG_ENTER();

    return (m_index += count);
}

void NumberOidIndexGenerator::reset()
{
    SWSS_LOG_ENTER();

    m_index = 0;
}
