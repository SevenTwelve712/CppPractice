#include "header.hpp"
#include <fstream>
#include <iostream>
#include <istream>
#include <string>
#include <vector>
namespace {
std::string slow_get_page(const int &key)
{
    return std::to_string(key);
}
class CacheTest
{
    TwoQueueCache::TwoQueueCache<int, std::string> cache;
    int in_hits;
    int out_hits;
    int hot_hits;
    int misses;

  public:
    CacheTest(int in, int out, int hot)
        : in_hits(0),
          out_hits(0),
          hot_hits(0),
          misses(0),
          cache(in, out, hot, slow_get_page) {};

    void warm_up(std::vector<int> &data)
    {
        for (int elem : data) {
            cache.get_elem(elem);
            // cache.validate_invariants();
        }
    }
    std::vector<int> test(std::vector<int> &data)
    {
        for (int elem : data) {
            const TwoQueueCache::CacheNode<int, std::string> *q = cache.get_cache_node(elem);
            if (q == nullptr) {
                misses++;
            }
            else if (q->queue == TwoQueueCache::Queues::hot) {
                hot_hits++;
            }
            else if (q->queue == TwoQueueCache::Queues::in) {
                in_hits++;
            }
            else if (q->queue == TwoQueueCache::Queues::out) {
                out_hits++;
            }
            cache.get_elem(elem);
            // cache.validate_invariants();
        }
        // std::cout << "Hot hits: " << hot_hits << std::endl;
        // std::cout << "In hits: " << in_hits << std::endl;
        // std::cout << "Out hits: " << out_hits << std::endl;
        return std::vector<int>({in_hits, out_hits, hot_hits, misses + hot_hits + in_hits + out_hits});
    }
};
} // namespace

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " filename" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        std::cout << "Error while reading " << filename << " file" << std::endl;
        return 1;
    }

    int in_size, out_size, hot_size, warm_up_size, test_size, in_hits, out_hits, hot_hits, total;

    if (!(fin >> in_size >> out_size >> hot_size >> warm_up_size >> test_size)) {
        std::cout << filename << " isn`t a test file" << std::endl;
        fin.close();
        return 1;
    }

    // reading data
    std::vector<int> warmup(warm_up_size);
    for (int i = 0; i < warm_up_size; i++) {
        if (!(fin >> warmup[i])) {
            std::cout << filename << " isn`t a test file" << std::endl;
            fin.close();
            return 1;
        }
    }

    std::vector<int> test(test_size);
    for (int i = 0; i < test_size; i++) {
        if (!(fin >> test[i])) {
            std::cout << filename << " isn`t a test file" << std::endl;
            fin.close();
            return 1;
        }
    }

    // reading metrics
    if (!(fin >> in_hits >> out_hits >> hot_hits >> total)) {
        std::cout << filename << " isn`t a test file" << std::endl;
        fin.close();
        return 1;
    }

    // test
    CacheTest cache_test(in_size, out_size, hot_size);
    cache_test.warm_up(warmup);
    std::vector<int> metrics_real = cache_test.test(test);

    if (metrics_real[0] == in_hits && metrics_real[1] == out_hits && metrics_real[2] == hot_hits && metrics_real[3] == total) {
        std::cout << "Test " << filename << " passed" << std::endl;
    }
    else {
        std::cout << "Test " << filename << " failed, expectes in hits: " << in_hits << "; out hits: " << out_hits << "; hot_hits: " << hot_hits
                  << "; got in_hits: " << metrics_real[0] << "; out_hits: " << metrics_real[1] << "; hot_hits: " << metrics_real[2] << std::endl;
    }
    fin.close();
    return 0;
}
