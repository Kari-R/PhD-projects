#include <Reach.hpp>

Reach::Reach(node_t N):
    reach(N, 0), almost_reach(N, 0) {}

bool Reach::has_cycle() const {
    for(node_t x = 0; x < reach.size(); x++)
        if(reach[x].test(x) || almost_reach[x].test(x))
            return true;
    return false;
}

void Reach::set_bidir_edge(node_t x, node_t y) {
    ensure(x < reach.size());
    ensure(y < reach.size());

    almost_reach[x].set(y);
    almost_reach[x] |= reach[y];

    almost_reach[y].set(x);
    almost_reach[y] |= reach[x];

    for(node_t z = 0; z < reach.size(); z++) {
        if(reach[z].test(x)) {
            almost_reach[z].set(y);
            almost_reach[z] |= reach[y]; /* z -> x <-> y -> o */
        }
        if(reach[z].test(y)) {
            almost_reach[z].set(x);
            almost_reach[z] |= reach[x]; /* o <- x <-> y <- z */
        }
    }
}

void Reach::set_parent(node_t y, node_t x) {  /* x -> y */
    ensure(x < reach.size());
    ensure(y < reach.size());
    ensure(x != y);

    reach[x].set(y);
    reach[x] |= reach[y];
    almost_reach[x] |= almost_reach[y];

    for(node_t z = 0; z < reach.size(); z++) {
        if(reach[z].test(x)) {  /* z -> x -> y */
            reach[z].set(y);
            reach[z] |= reach[y];               /* z -> x -> y -> o */
            almost_reach[z] |= almost_reach[y]; /* z -> x -> y <-> o */
        }
        if(almost_reach[z].test(x)) { /* z <-> x -> y */
            almost_reach[z].set(y);
            almost_reach[z] |= reach[y]; /* z <-> x -> y -> o */
        }
    }
}

void Reach::simplify_past(bits_t future, bool extra) {
    for(node_t x = 0; x < reach.size(); x++) {
        if(!future.test(x)) {
            reach[x].reset();
            almost_reach[x].reset();
        }
        if(extra) almost_reach[x] &= ~reach[x];
    }
}

Reach::Key::Key(bits_t remain, const Reach& r, bool optimize) {
    data.push_back(0);
    data.push_back(remain.to_ulong());
    size_t hash = 0;
    for(node_t i = 0; i < r.reach.size(); i++)
        if(!optimize || remain.test(i)) {
            data.push_back(r.reach[i].to_ulong());
            hash ^= std::hash<int>{}( data.back() ) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
    for(node_t i = 0; i < r.almost_reach.size(); i++)
        if(!optimize || remain.test(i)) {
            data.push_back(r.almost_reach[i].to_ulong());
            hash ^= std::hash<int>{}( data.back() ) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
    data[0] = hash;
}

bool Reach::Key::operator==(const Key& key) const {
    if(data.size() != key.data.size())
        return false;
    for(size_t i = 0; i < data.size(); i++)
        if(data[i] != key.data[i])
            return false;
    return true;
}
