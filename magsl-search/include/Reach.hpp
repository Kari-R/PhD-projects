#pragma once

#include <util.hpp>

class Reach {
public:
    Reach(node_t);

    bool has_cycle() const;
    void set_bidir_edge(node_t, node_t);
    void set_parent(node_t, node_t);
    void simplify_past(bits_t, bool extra = false);
    
    inline bool has(node_t x, node_t y) const {
        return reach[x].test(y);
    }

    struct Key {
        std::vector<size_t> data;
        
        Key(bits_t, const Reach&, bool optimize = false);
        
        inline size_t hash() const { return data[0]; }
        bool operator==(const Key&) const;
    };

private:
    std::vector<bits_t> reach, almost_reach;
};

namespace std {
    template<>
    struct hash<Reach::Key> {
        inline size_t operator()(const Reach::Key& key) const {
            return key.hash();
        }
    };
}
