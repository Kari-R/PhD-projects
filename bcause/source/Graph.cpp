#include <Graph.hpp>

Graph::Graph(const Instance& i): instance(&i),
    degree(i.size(), 0),
    pairs(i.size() * i.size(), 0),
    bidir_count(0) {
    reach.push_back(BitMatrix(instance->size()));
}

void Graph::decide(const Edge& decision) {
    get(decision.x(), decision.y()).set(decision.type());

    if(decision.present()) {
        degree[decision.x()]++;
        degree[decision.y()]++;
        if(decision.is_HH()) bidir_count++;
        
        if(decision.is_TH() || decision.is_HT()) {
            reach.push_back(reach.back());

            reach.back()(decision.cause()) |= reach.back()(decision.effect());
            reach.back().set(decision.cause(), decision.effect());

            for(uchar v = 0; v < instance->size(); v++)
                if(reach.back()(v, decision.cause()))
                    reach.back()(v) |= reach.back()(decision.cause());
        }
    }
}

void Graph::undecide(const Edge& decision) {
    get(decision.x(), decision.y()).reset(decision.type());

    if(decision.present()) {
        degree[decision.x()]--;
        degree[decision.y()]--;
        if(decision.is_HH()) bidir_count--;
        
        if(decision.is_TH() || decision.is_HT())
            reach.pop_back();
    }
}

void Graph::output() const {
    print("Graph:");
    for(uchar x = 0; x < instance->size(); x++)
        for(uchar y = 0; y < instance->size(); y++) {
            if(x != y && edges(x, y))
                print(" %->%", (int)x + instance->variable_indexing(), (int)y + instance->variable_indexing());
            if(x < y && biedges(x, y))
                print(" %<>%", (int)x + instance->variable_indexing(), (int)y + instance->variable_indexing());
        }
    print("\n");
}
