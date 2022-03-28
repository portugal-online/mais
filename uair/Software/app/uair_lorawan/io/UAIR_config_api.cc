#include "UAIR_config_api.h"

#include <array>
#include <vector>

namespace
{
    template<class T>
    class Cache{
        struct Item {
            T value;
            bool valid;
        };
        size_t m_max_size{ 0 };
        std::vector<Item> m_elems;

    public:
        Cache() = default;
        explicit Cache(size_t max_size) : m_max_size{ max_size }
        {}

        size_t max_size() const noexcept {return m_max_size;}

        bool push() noexcept
        {
            if (m_elems.empty())
            {
                try{  m_elems.resize(m_max_size); }
                catch(...){ return false; }

                for (Item& item : m_elems)
                    item.valid = false;
            }
        }
    };

    Cache<uint8_t> s_cache;

    std::array<uair_config_pair_uint8, 2> s_config_default = {{
        { UAIR_CONFIG_ID_TX_POLICY, 1 },
        { UAIR_CONFIG_ID_FAIR_RATIO, 1 }
    }};
}

int8_t uair_config_cache_size(int8_t new_size)
{
    if (new_size >= 0)
        s_cache = Cache<uint8_t>{static_cast<size_t>(new_size)};
    
    return static_cast<int8_t>(s_cache.max_size());
}

int uair_config_write_uint8(uair_config_id id, uint8_t value)
{
    return 0;
}

int uair_config_read_uint8(uair_config_id id, uint8_t *value)
{
    return 0;
}

const uair_config_pair_uint8* config_defaults_uint8(int* size)
{
    if (size)
        *size = static_cast<int>(s_config_default.size());
    return s_config_default.data();
}

int uair_config_write_uint8s(const uair_config_pair_uint8* pairs, int size)
{
    if (!pairs || (size <= 0)) return 0;
    return 0;
}

int uair_config_read_uint8s(uair_config_pair_uint8* pairs, int size)
{
    if (!pairs || (size <= 0)) return 0;

    return 0;
}
