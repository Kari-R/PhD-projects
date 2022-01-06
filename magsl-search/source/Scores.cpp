#include <Scores.hpp>
#include <sstream>

bits_t get_subset(std::istream& in) {
    size_t value;
    in >> value;
    return bits_t(value);
}

Scores::Scores(std::istream& stream) {
    stream >> max_vars >> max_parents >> max_component;

    map = std::vector<std::unordered_map<Comparent::Key, size_t> >(
        size_t(1) << max_vars, std::unordered_map<Comparent::Key, size_t>() );

    BENCHMARK("reading the scores",
        while(!stream.eof()) {
	    std::string str;
	    getline(stream, str);
	    if(str.length() <= 2) continue;

	    std::stringstream stream2(str);
            score_t score;
            stream2 >> score;
	    score *= -1;

            Comparent scoreable( get_subset(stream2), score );
            for(size_t i = 0; i < scoreable.component().size(); i++)
                scoreable.add_parents( get_subset(stream2) );
	    while(!stream2.eof())
            	scoreable.add_missing_edge( get_subset(stream2) );
            comparents.push_back(scoreable);

            map[scoreable.component().bitset().to_ulong()].insert(
                std::make_pair(Comparent::Key(scoreable), comparents.size() - 1));
        }
    );
    print("Score-Count: %\n", comparents.size());
}

void Scores::preprocess(const Config& config) {
    for(size_t id = 0; id < comparents.size(); id++) {
        auto it = by_comp.find(comparents[id].component().bitset().to_ulong());
        if(it == by_comp.end())
            it = by_comp.insert(by_comp.begin(),
                std::make_pair(comparents[id].component().bitset().to_ulong(), std::vector<size_t>()));
        it->second.push_back(id);
        
        to_score.insert({ Comparent::Key(comparents[id]), comparents[id].score() });
    }
}

void Scores::get_partitions(std::vector<bits_t>& partitions, bits_t subset, bits_t vars, node_t i) {
    if(i >= size() || subset.count() + 1 == vars.count()) {
        if(subset.count() > 0)
            partitions.push_back(subset);
        return;
    }
    get_partitions(partitions, subset, vars, i + 1);
    if(vars.test(i))
        get_partitions(partitions, subset.set(i), vars, i + 1);
}

void Scores::copy_parents(Comparent::Key& key, const Comparent::Key& source) {
    node_t i = 0, j = 0;
    for(node_t node = 0; node < size(); node++) {
        if(bits_t(key.component()).test(node)) {
            key.set_parents(i, source.parents(j));
            i++;
        }
        if(bits_t(source.component()).test(node))
            j++;
    }
}

void Scores::output_to(std::ostream& os) const {
    os << max_vars << " " << max_parents << " " << max_component << std::endl;
    for(const Comparent& comparent: comparents)
        comparent.output_to(os);
}

const std::vector<size_t>& Scores::by_component(bits_t bits) const {
    static std::vector<size_t> empty;
    auto it = by_comp.find(bits.to_ulong());
    if(it == by_comp.end())
        return empty;
    return it->second;
}

void Scores::gather_subsets(std::vector<bits_t>& vec, bits_t bits, bits_t remain, size_t node) const {

    if(node >= size()) {
        if(bits.count() > 0)
            vec.push_back(bits);
        return;
    }

    gather_subsets(vec, bits, remain, node + 1);
    if(bits.count() < component_limit() && remain.test(node))
        gather_subsets(vec, bits.set(node), remain, node + 1);
}
