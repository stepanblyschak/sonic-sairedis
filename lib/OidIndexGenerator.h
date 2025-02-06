#pragma once

#include <stdint.h>

namespace sairedis
{
    class OidIndexGenerator
    {
        public:

            OidIndexGenerator() = default;

            virtual ~OidIndexGenerator() = default;

        public:

            virtual uint64_t increment(uint64_t count = 1) = 0;

            virtual void reset() = 0;
    };
}
