#pragma once

#include "lib/OidIndexGenerator.h"

namespace saimeta
{
    class NumberOidIndexGenerator:
        public sairedis::OidIndexGenerator
    {

        public:

            NumberOidIndexGenerator();

            virtual ~NumberOidIndexGenerator() = default;

        public:

            virtual uint64_t increment(uint64_t count = 1) override;

            virtual void reset() override;

        private:

            uint64_t m_index;
    };
}
