#pragma once
#include <iostream>
#include <list>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <unordered_map>
namespace TwoQueueCache {
enum Queues
{
    in,
    out,
    hot,
    none
};

template <typename Key, typename Value>
struct CacheNode
{
    typename std::list<Key>::iterator iter;
    Queues queue;
    std::optional<Value> val;
};

template <typename Key, typename Value>
class TwoQueueCache
{
  public:
    using GetPage = Value (*)(const Key &);

  private:
    std::list<Key> in;
    std::list<Key> out;
    std::list<Key> hot;
    std::unordered_map<Key, CacheNode<Key, Value>> table;

    int IN_SIZE;
    int OUT_SIZE;
    int HOT_SIZE;

    GetPage slow_get_page;

    static void validate(int in, int out, int hot, GetPage slow_get_page)
    {
        if (in <= 0 || out <= 0 || hot <= 0 || slow_get_page == nullptr) {
            throw std::invalid_argument("Cache sizes must be more than zero and slow_get_page can`t be nullptr");
        }
    }

  public:
    TwoQueueCache(int in_size, int out_size, int hot_size, GetPage slow_get_page);
    bool validate_invariants() const;
    const CacheNode<Key, Value> *get_cache_node(const Key &key) const;
    void get_elem(const Key &key);

    // getters
    int get_in_size() const
    {
        return IN_SIZE;
    }
    int get_out_size() const
    {
        return OUT_SIZE;
    }
    int get_hot_size() const
    {
        return HOT_SIZE;
    }

    int get_curr_in_size() const
    {
        return in.size();
    }
    int get_curr_hot_size() const
    {
        return hot.size();
    }
    int get_curr_out_size() const
    {
        return out.size();
    }
    int get_table_size() const
    {
        return table.size();
    }
};
} // namespace TwoQueueCache

template <typename Key, typename Value>
TwoQueueCache::TwoQueueCache<Key, Value>::TwoQueueCache(int in_size, int out_size, int hot_size, GetPage slow_get_page)
    : IN_SIZE((validate(in_size, out_size, hot_size, slow_get_page), in_size)),
      HOT_SIZE(hot_size),
      OUT_SIZE(out_size),
      slow_get_page(slow_get_page)
{
    table.reserve(in_size + hot_size + out_size);
}

template <typename Key, typename Value>
const TwoQueueCache::CacheNode<Key, Value> *
TwoQueueCache::TwoQueueCache<Key, Value>::get_cache_node(const Key &key) const
{
    auto it = table.find(key);
    if (it != table.end()) {
        return &(it->second);
    }

    return nullptr;
}

template <typename Key, typename Value>
void TwoQueueCache::TwoQueueCache<Key, Value>::get_elem(const Key &key)
{
    const CacheNode<Key, Value> *qp = get_cache_node(key);
    if (qp == nullptr) {
        if (in.size() == IN_SIZE) {
            if (out.size() == OUT_SIZE) {
                Key out_last = out.back();
                out.pop_back();
                table.erase(out_last);
            }
            out.splice(out.begin(), in, std::prev(in.end()));
            table[*out.begin()].val = std::nullopt;
            table[*out.begin()].queue = Queues::out;
        }
        in.push_front(key);
        table.insert({key, CacheNode<Key, Value>{in.begin(), Queues::in, std::optional<Value>(slow_get_page(key))}});
    }

    else if (qp->queue == Queues::out) {
        // std::cout << "Elem in out queue" << std::endl;
        if (hot.size() == HOT_SIZE) {
            Key hot_last = hot.back();
            table.erase(hot_last);
            hot.pop_back();
        }
        hot.splice(hot.begin(), out, qp->iter);
        table[*hot.begin()].queue = Queues::hot;
        table[*hot.begin()].val = slow_get_page(key);
    }
    // std::cout << "Elem inserted" << std::endl;
    // std::cout << "---------------" << std::endl;
    //  std::cout << "Totl elems " << in.size() + out.size() + hot.size() << std::endl;
}

template <typename Key, typename Value>
bool TwoQueueCache::TwoQueueCache<Key, Value>::validate_invariants() const
{
    // проверяем уникальность значений в кэшах
    for (Key key : in) {
        for (Key key_ : out) {
            if (key == key_) {
                std::cout << "Find same key in in, out: " << key << std::endl;
                return false;
            }
            for (Key key_ : hot) {
                if (key == key_) {
                    std::cout << "Find same key in in, hot: " << key << std::endl;
                    return false;
                }
            }
        }
    }

    for (Key key : out) {
        for (Key key_ : hot) {
            if (key == key_) {
                std::cout << "Find same key in out, hot: " << key << std::endl;
                return false;
            }
        }
    }

    // проверяем количество ключей в таблице
    int keys_real = table.size(), keys_expected = in.size() + out.size() + hot.size();
    if (keys_expected != keys_real) {
        std::cout << "Expected " << keys_expected << " in table; get " << keys_real << std::endl;
        return false;
    }
    return true;
}
