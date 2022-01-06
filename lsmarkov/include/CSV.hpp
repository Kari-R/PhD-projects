#pragma once

#include <Bitset.hpp>
#include <util.hpp>
#include <map>
#include <fstream>
#include <vector>
#include <unordered_map>

class CSV {
public:
    using value_t = unsigned char;
    using Sample = std::vector<value_t>;
    using ValueMap = std::unordered_map<std::string, value_t>;
    
    void add(size_t, const std::string&);
    size_t read_sample(const std::string&);

    CSV(std::ifstream&);
    
    inline size_t arity(size_t var) const {
        ensure(var < values.size());
        return values[var].size();
    }

    inline size_t size() const { return samples.size(); }
    inline size_t variables() const { return values.size(); }

    inline const Sample& operator[](size_t i) const {
        ensure(i < samples.size());
        return samples[i];
    }

private:
    std::vector<ValueMap> values;
    std::vector<Sample> samples;
};
