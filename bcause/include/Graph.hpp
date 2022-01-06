#pragma once

#include <Instance.hpp>
#include <BitMatrix.hpp>
#include <vector>

class Edge {
public:
    enum Type {
        HH_PRESENT,
        TH_PRESENT,
        HT_PRESENT,
        HH_ABSENT,
        TH_ABSENT,
        HT_ABSENT,
    };
    
    Edge(uchar x, uchar y):
        Edge(x, y, x < y? TH_PRESENT: HT_PRESENT)
    {}
    
    Edge(uchar x, uchar y, Type type) {
        if(x > y)
            std::swap(x, y);
        data = size_t(x) +
            (size_t(y) << 8) +
            (size_t(type) << 16);
    }
    
    inline uchar x() const { return data & 0xFF; };
    inline uchar y() const { return (data & (0xFF << 8)) >> 8; }
    
    inline Type type() const { return (Type)((data & (0xFF << 16)) >> 16); }
    
    inline bool present() const { return type() < HH_ABSENT; }
    inline bool absent() const { return type() >= HH_ABSENT; }
    
    inline bool is_HH() const { return type() == HH_ABSENT || type() == HH_PRESENT; }
    inline bool is_TH() const { return type() == TH_ABSENT || type() == TH_PRESENT; }
    inline bool is_HT() const { return type() == HT_ABSENT || type() == HT_PRESENT; }
    
    inline bool is_TH(uchar cause, uchar effect) const {
        return (x() == cause && y() == effect && is_TH()) ||
            (x() == effect && y() == cause && is_HT());
    }
    
    inline bool is_HH(uchar cause, uchar effect) const {
        if(cause > effect)
            return is_HH(effect, cause);
        return is_HH() && x() == cause && y() == effect;
    }
    
    const std::string string() const {
        const std::string symbols[] { "<->", "-->", "<--", "< >", "  >", "<  " };
        return format("% % %", (int)x() + 1, symbols[(int)type()], (int)y() + 1);
    }
    
    inline Edge negation() const {
        return present()? Edge(x(), y(), (Type)((uchar)type() + 3)):
            Edge(x(), y(), (Type)((uchar)type() - 3));
    }
    
    inline uchar cause() const {
        ensure(!is_HH());
        return is_TH()? x(): y();
    }
    
    inline uchar effect() const {
        ensure(!is_HH());
        return is_TH()? y(): x();
    }

private:
    size_t data;
};

class Completion;

class Graph {
public:
    inline bool edge_decided(uchar x, uchar y) const {
        return x < y? (get(x, y).test(Edge::TH_PRESENT) || get(x, y).test(Edge::TH_ABSENT)):
                (get(y, x).test(Edge::HT_PRESENT) || get(y, x).test(Edge::HT_ABSENT));
    }
    
    inline bool biedge_decided(uchar x, uchar y) const {
        auto& data = x < y? get(x, y): get(y, x);
        return data.test(Edge::HH_PRESENT) || data.test(Edge::HH_ABSENT);
    }
    
    inline bool is_decidable(const Edge& decision) const {
        auto& edges = get(decision.x(), decision.y());
        return !edges.test((uchar)decision.type()) &&
               !edges.test((uchar)decision.negation().type());
    }

    inline bool edges(uchar x, uchar y) const {
        return x < y? get(x, y).test(Edge::TH_PRESENT):
            get(y, x).test(Edge::HT_PRESENT);
    }
    
    inline bool biedges(uchar x, uchar y) const {
        return x < y? get(x, y).test(Edge::HH_PRESENT):
            get(y, x).test(Edge::HH_PRESENT);
    }
    
    void decide(const Edge&);
    void undecide(const Edge&);
    
    explicit Graph(const Instance&);

    inline uchar size() const { return instance->size(); }

    inline bool could_add_more_edges(uchar x, uchar y) const {
        return (*instance)[MaxDegree] == NotLimited || (
            degree[x] < (size_t)(*instance)[MaxDegree] &&
            degree[y] < (size_t)(*instance)[MaxDegree]
        );
    }
    
    inline size_t bidirected_count() const { return bidir_count; }
    
    void output() const;

    inline bool requires(Argument arg) const {
        return (*instance)[arg];
    }

    inline bool retains_acyclicity(uchar x, uchar y) const {
        return !reach.back()(y, x);
    }
    
    inline bool has_directed_path(uchar a, uchar b) const {
        return reach.back()(a, b);
    }

    inline size_t min_degree(uchar v) const { return degree[v]; }
    
    friend class Completion;
    
protected:
    inline std::bitset<8>& get(uchar x, uchar y) {
        return pairs[x + y * instance->size()];
    }
    
    inline const std::bitset<8>& get(uchar x, uchar y) const {
        return pairs[x + y * instance->size()];
    }

private:
    const Instance* instance;
    std::vector<size_t> degree;
    std::vector<std::bitset<8> > pairs;
    std::vector<BitMatrix> reach;
    size_t bidir_count;
};

#include <functional>

const bool MINIMAL = false;
const bool MAXIMAL = true;

class Completion {
public:
    typedef std::function<bool(Edge)> Fun;
    
    static bool default_fun(Edge) { return false; }
    
    template<typename Function>
    Completion(const Graph& solution, bool compl_type, const Function& fun):
        solution(solution), compl_type(compl_type), known_independence(fun) {}
    
    Completion(const Graph& solution, bool compl_type):
        Completion(solution, compl_type, &Completion::default_fun) {}
    
    inline bool requires(Argument arg) const {
        return solution.requires(arg);
    }

    inline bool edges(uchar x, uchar y) const {
        return solution.edges(x, y) ||
            (compl_type == MAXIMAL && !solution.edge_decided(x, y) &&
                virtual_edge(x, y));
    }
    
    inline bool biedges(uchar x, uchar y) const {
        if(solution.biedges(x, y))
            return true;
        if(compl_type != MAXIMAL || solution.biedge_decided(x, y))
            return false;
        if(solution.bidirected_count() >= (size_t)(*solution.instance)[MaxBidirCount])
            return false;
        return virtual_biedge(x, y);
    }
    
    inline uchar size() const { return solution.size(); }
    inline bool type() const { return compl_type; }
    
    inline void output() const { solution.output(); }
    
protected:
    inline bool virtual_edge(uchar x, uchar y) const {
        return solution.could_add_more_edges(x, y) &&
            !known_independence(Edge(x, y));
    }
    
    inline bool virtual_biedge(uchar x, uchar y) const {
        return solution.could_add_more_edges(x, y) &&
            !known_independence(Edge(x, y, Edge::HH_PRESENT));
    }
    
private:
    const Graph& solution;
    const bool compl_type;
    Fun known_independence;
};
