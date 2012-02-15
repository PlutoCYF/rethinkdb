#ifndef __UNITTEST_DUMMY_PROTOCOL_HPP__
#define __UNITTEST_DUMMY_PROTOCOL_HPP__

#include <map>
#include <set>
#include <string>
#include <vector>

#include "utils.hpp"
#include <boost/function.hpp>
#include <boost/serialization/set.hpp>

#include "concurrency/fifo_checker.hpp"
#include "concurrency/rwi_lock.hpp"
#include "protocol_api.hpp"
#include "rpc/serialize_macros.hpp"
#include "timestamps.hpp"

class signal_t;

namespace unittest {

class dummy_protocol_t {

public:

    class region_t {
    public:
        static region_t empty() THROWS_NOTHING;
        RDB_MAKE_ME_SERIALIZABLE_1(keys);
        std::set<std::string> keys;
        bool operator<(const region_t &other) const;
    };

    class temporary_cache_t {
        /* Dummy protocol doesn't need to cache anything */
    };

    class read_response_t {
    public:
        RDB_MAKE_ME_SERIALIZABLE_1(values);

        std::map<std::string, std::string> values;
    };

    class read_t {
    public:
        region_t get_region() const;
        read_t shard(region_t region) const;
        read_response_t unshard(std::vector<read_response_t> resps, temporary_cache_t *cache) const;
        RDB_MAKE_ME_SERIALIZABLE_1(keys);
        region_t keys;
    };

    class write_response_t {
    public:
        RDB_MAKE_ME_SERIALIZABLE_1(old_values);
        std::map<std::string, std::string> old_values;
    };

    class write_t {
    public:
        region_t get_region() const;
        write_t shard(region_t region) const;
        write_response_t unshard(std::vector<write_response_t> resps, temporary_cache_t *cache) const;
        RDB_MAKE_ME_SERIALIZABLE_1(values);
        std::map<std::string, std::string> values;
    };

    class backfill_chunk_t {
    public:
        std::string key, value;
        state_timestamp_t timestamp;

        RDB_MAKE_ME_SERIALIZABLE_3(key, value, timestamp);
    };
};

bool region_is_superset(dummy_protocol_t::region_t a, dummy_protocol_t::region_t b);
dummy_protocol_t::region_t region_intersection(dummy_protocol_t::region_t a, dummy_protocol_t::region_t b);
dummy_protocol_t::region_t region_join(const std::vector<dummy_protocol_t::region_t>& vec) THROWS_ONLY(bad_join_exc_t, bad_region_exc_t);
std::vector<dummy_protocol_t::region_t> region_subtract_many(dummy_protocol_t::region_t a, const std::vector<dummy_protocol_t::region_t>& b);

bool operator==(dummy_protocol_t::region_t a, dummy_protocol_t::region_t b);
bool operator!=(dummy_protocol_t::region_t a, dummy_protocol_t::region_t b);

class dummy_underlying_store_t {

public:
    explicit dummy_underlying_store_t(dummy_protocol_t::region_t r);

    dummy_protocol_t::region_t region;

    std::map<std::string, std::string> values;
    std::map<std::string, state_timestamp_t> timestamps;

    region_map_t<dummy_protocol_t, binary_blob_t> metadata;
};

class dummy_store_view_t : public store_view_t<dummy_protocol_t> {

public:
    dummy_store_view_t(dummy_underlying_store_t *p, dummy_protocol_t::region_t region);

    boost::shared_ptr<store_view_t<dummy_protocol_t>::read_transaction_t> begin_read_transaction(signal_t *interruptor) THROWS_ONLY(interrupted_exc_t);
    boost::shared_ptr<store_view_t<dummy_protocol_t>::write_transaction_t> begin_write_transaction(signal_t *interruptor) THROWS_ONLY(interrupted_exc_t);

private:
    friend class dummy_transaction_t;
    dummy_underlying_store_t *parent;
    rwi_lock_t read_write_lock;
};

dummy_protocol_t::region_t a_thru_z_region();

}   /* namespace unittest */

#endif /* __UNITTEST_DUMMY_PROTOCOL_HPP__ */