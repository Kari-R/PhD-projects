#include <CSV.hpp>

void CSV::add(size_t var, const std::string& value) {
    if(values.size() <= var)
        values.push_back(ValueMap());

    ValueMap& map = values[var];
    auto it = map.find(value);
    if(it == map.end())
        it = map.insert(map.begin(), std::make_pair(value, map.size()));
    samples.back().push_back(it->second);
}

size_t CSV::read_sample(const std::string& s) {
    size_t var = 0;
    std::string value;
    samples.push_back(Sample());
    for(size_t i = 0; i < s.length(); i++) {
        if(s[i] == ' ' || s[i] == ',') {
            add(var, value);
            value = {};
            var++;
        } else
            value += s[i];
    }
    return var;
}

CSV::CSV(std::ifstream& in) {
    ensure(in.is_open());
    size_t prev_count = 0;
    while(!in.eof()) {
        std::string line;
        getline(in, line);
        if(line.empty())
            continue;
        size_t count = read_sample(line + ",");
        if(Bitset::capacity() < count)
            Bitset::capacity() = count;
        if(!prev_count)
            prev_count = count;
        else { ensure(count == prev_count); }
    }
}
