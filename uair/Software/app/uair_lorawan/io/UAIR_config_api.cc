#include "UAIR_config_api.h"

#include <array>
#include <cassert>

namespace
{
    template<class T>
    class Cache{
        struct Item {
            T item;
            bool valid;
        };
        size_t m_max_size{ 0 };
        Item* m_data{ nullptr };

    public:
        Cache() = default;
        explicit Cache(size_t max_size) : m_max_size{ max_size }
        {
            if (m_max_size <= 0) return;

            m_data = (Item*)malloc(sizeof(Item) * m_max_size);
            if (!m_data) return;

            for (size_t i = 0; i < m_max_size; i++)
                m_data[i].valid = false;
        }

        Cache(const Cache&) = delete;
        Cache(Cache&&) = delete;
        Cache& operator=(const Cache&) = delete;
        Cache& operator=(Cache&& other) noexcept
        {
            std::swap(m_data, other.m_data);
            std::swap(m_max_size, other.m_max_size);
            return *this;
        }

        ~Cache()
        {
            free(m_data);
            m_max_size = 0;
            m_data = nullptr;
        }

        explicit operator bool() const noexcept {return m_data;}
        size_t max_size() const noexcept {return m_max_size;}

        bool push(T item) noexcept
        {
            if (!m_data) return false;

            assert(m_max_size >= 1);
            if (m_max_size >= 2) //move everything forward
            {
                for (auto walker = m_data + m_max_size - 2; walker >= m_data; --walker)
                    walker[1] = walker[0];
            }

            m_data[0].item = item;
            m_data[0].valid = true;
            return true;
        }

        template<class TPredicate>
        bool has_value(TPredicate&& p) const
        {
            if (!m_data) return false;

            auto walker = m_data;
            auto end = m_data + m_max_size;

            for (; walker < end; ++walker)
            {
                if (!walker->valid) return false;
                if (p(walker->item)) break; //found it
            }

            return (walker < end);
        }

        template<class TPredicate>
        bool get_value(TPredicate&& p)
        {
            if (!m_data) return false;

            auto walker = m_data;
            auto end = m_data + m_max_size;

            for (; walker < end; ++walker)
            {
                if (!walker->valid) return false;
                if (p(walker->item)) break; //found it
            }

            if (walker >= end) return false;

            //found it, so move item to the beginning

            if (walker == m_data) return true;

            auto target = *walker;
            for (--walker; walker >= m_data; --walker)
                walker[1] = walker[0];
            m_data[0] = target;

            return true;
        }
    };

    Cache<uair_config_pair_uint8> s_cache;

    std::array<uair_config_pair_uint8, 2> s_config_default = {{
        { UAIR_CONFIG_ID_TX_POLICY, 1 },
        { UAIR_CONFIG_ID_FAIR_RATIO, 1 }
    }};

    void cache_update(uair_config_id id, uint8_t value)
    {
        if (s_cache.has_value([&id](const uair_config_pair_uint8& item){ return (item.id == id); }))
            return;

        uair_config_pair_uint8 item;
        item.id = id;
        item.value = value;
        s_cache.push(item);
    }

    bool cache_read(uair_config_id id, uint8_t *value)
    {
        auto pred = [&id, &value](const uair_config_pair_uint8& item)
        {
            if (item.id != id) return false;

            if (value) *value = item.value;
            return true;
        };

        return s_cache.get_value(pred);
    }

    template<class TCallback>
    void io_write(uair_io_context& ctx, TCallback&& c)
    {
        c(ctx);
        if ((ctx.error == UAIR_IO_CONTEXT_ERROR_CTX_CHECK) && (ctx.flags == UAIR_IO_CONTEXT_FLAG_FLUSH))
        {
            UAIR_io_config_flush(&ctx);
            c(ctx);
        }
    }
}

#ifdef UNITTESTS
size_t g_config_api_flash_num_reads = 0;
#endif

int8_t uair_config_cache_size(int8_t new_size)
{
    if (new_size >= 0)
    {
        s_cache = Cache<uair_config_pair_uint8>{static_cast<size_t>(new_size)};
        if ((new_size > 0) && !s_cache)
            s_cache = Cache<uair_config_pair_uint8>();
    }
    
    return static_cast<int8_t>(s_cache.max_size());
}

int uair_config_write_uint8(uair_config_id id, uint8_t value)
{
    uair_io_context ctx;
    UAIR_io_init_ctx(&ctx);

    io_write(ctx, [&id, &value](uair_io_context& ctx){ UAIR_io_config_write_uint8(&ctx, (uair_io_context_keys)id, value); });
    if (ctx.error) return ctx.error;

    cache_update(id, value);
    return UAIR_IO_CONTEXT_ERROR_NONE;
}

int uair_config_read_uint8(uair_config_id id, uint8_t *value)
{
    if (cache_read(id, value))
        return UAIR_IO_CONTEXT_ERROR_NONE;
    
    uair_io_context ctx;
    UAIR_io_init_ctx(&ctx);

    uint8_t stored_value;
    UAIR_io_config_read_uint8(&ctx, (uair_io_context_keys)id, &stored_value);
    if (ctx.error)
        return ctx.error;

#ifdef UNITTESTS
    g_config_api_flash_num_reads++;
#endif

    cache_update(id, stored_value);
    if (value) *value = stored_value;

    return UAIR_IO_CONTEXT_ERROR_NONE;
}

const uair_config_pair_uint8* config_defaults_uint8(int* size)
{
    if (size)
        *size = static_cast<int>(s_config_default.size());
    return s_config_default.data();
}

int uair_config_write_uint8s(const uair_config_pair_uint8* pairs, int size)
{
    if (!pairs || (size <= 0))
        return UAIR_IO_CONTEXT_ERROR_NONE;

    uair_io_context ctx;
    UAIR_io_init_ctx(&ctx);

    auto p_walker = pairs;
    for (; size > 0; ++p_walker, --size)
    {
        io_write(ctx, [&p_walker](uair_io_context& ctx){ UAIR_io_config_write_uint8(&ctx, (uair_io_context_keys)p_walker->id, p_walker->value); });
        if (ctx.error) return ctx.error;

        cache_update(p_walker->id, p_walker->value);
    }

    return UAIR_IO_CONTEXT_ERROR_NONE;
}

int uair_config_read_uint8s(uair_config_pair_uint8* pairs, int size)
{
    if (!pairs || (size <= 0))
        return UAIR_IO_CONTEXT_ERROR_NONE;

    uair_io_context ctx;
    UAIR_io_init_ctx(&ctx);

    for (; size > 0; ++pairs, --size)
    {
        if (cache_read(pairs->id, &pairs->value))
            continue;
        
        UAIR_io_config_read_uint8(&ctx, (uair_io_context_keys)pairs->id, &pairs->value);
        if (ctx.error)
            return ctx.error;

#ifdef UNITTESTS
        g_config_api_flash_num_reads++;
#endif

        cache_update(pairs->id, pairs->value);
    }

    return UAIR_IO_CONTEXT_ERROR_NONE;
}
