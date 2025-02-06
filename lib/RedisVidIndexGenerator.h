#pragma once

#include "OidIndexGenerator.h"

#include "swss/dbconnector.h"
#include "swss/sal.h"

#include <memory>

namespace sairedis
{
    class RedisVidIndexGenerator:
        public OidIndexGenerator
    {
        public:

            RedisVidIndexGenerator(
                    _In_ std::shared_ptr<swss::DBConnector> dbConnector,
                    _In_ const std::string& vidCounterName);

            virtual ~RedisVidIndexGenerator() = default;

        public:

            virtual uint64_t increment(uint64_t count = 1) override;

            virtual void reset() override;

        private:

            std::shared_ptr<swss::DBConnector> m_dbConnector;

            std::string m_vidCounterName;
    };
}
