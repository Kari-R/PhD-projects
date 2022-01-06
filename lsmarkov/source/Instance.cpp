#include <Instance.hpp>
#include <set>
#include <Scorer.hpp>
#include <Graph.hpp>

Clique parse_clique(const std::string& s) {
    Clique clique;
    std::string cl;
    for(size_t i = 0; i < s.length(); i++) {
        if(s[i] == ',') {
            clique.set(to_number<size_t>(cl));
            cl = "";
        } else
            cl += s[i];
    }
    clique.set(to_number<size_t>(cl));
    return clique;
}

void read_subsets(std::set<unsigned long long>& set, Bits bits, size_t N, size_t tw) {
    if(bits.count() > tw)
        return;
    if(set.find(bits.to_ullong()) != set.end())
        return;
    set.insert(bits.to_ullong());
    for(size_t i = 0; i < N; i++)
        if(!bits.test(i))
            read_subsets(set, Bits(bits).set(i), N, tw);
}

void Instance::read_score_file() {
    std::ifstream file(config.filename());
    if(!file.is_open()) {
        print("Can't open '" + config.filename() + "'\n");
        std::exit(-1);
    }
    read_line(file);
    N = read_line<size_t>(file);
    Bitset::capacity() = N;
    if(N > Bits().size()) {
        print("Too many variables. Only N<=% supported at the moment.\n", Bits().size());
        std::exit(-1);
    }
    
    print("Reading score file...\n");
    read_line(file);
    std::string tw_line;
    getline(file, tw_line);
    size_t start = 0;
    while(tw_line[start] != ' ') start++;
    treewidth = to_number<size_t>(tw_line.substr(start + 1, tw_line.length()));
    if(config[MAX_CLIQUE].number > (long)treewidth) {
        print("The score file has lower treewidth (%) than what was requested (%).\n",
            treewidth, config[MAX_CLIQUE].number);
        std::exit(-1);
    }    
    std::set<unsigned long long> subsets;
    read_subsets(subsets, Bits(), N, treewidth);
    
    if(config[MAX_CLIQUE].number != -1) {
        print("Limiting treewidth is only for CSV files at the moment.\n");
        std::exit(-1);
    }
    
    for(Bits clique: subsets)
        if(clique.count() <= treewidth)
            scores.put(clique, read_line<score_t>(file));
    print("Score file is read. Instance has N=%, w=%\n", N, treewidth);
}

long double binomial(long double n, long double k) {
    return k == 0? 1:
        n * binomial(n - 1, k - 1) / k;
}

size_t clique_count(size_t N, size_t tw) {
    size_t count = 1;
    for(size_t k = 1; k <= tw; k++)
        count += binomial(N, k);
    return count;
}

size_t Instance::precompute_scores(size_t tw) {
    if(tw > treewidth) tw = treewidth;
    if(tw < 2) return 0;
    print("Computing % clique scores...\n", clique_count(max_size(), tw));
    return scores.precompute(tw);
}

void Instance::read_CSV_file() {
    std::ifstream file(config.filename());
    if(!file.is_open()) {
        print("Unable to open '" + config.filename() + "'\n");
        std::exit(-1);
    }
    print("Reading CSV file...\n");
    CSV* csv = new CSV(file);
    scores.to_be_computed_using(csv, config[ESS].number);
    N = csv->variables();
    Bitset::capacity() = N;
    if(config[MAX_CLIQUE].number > N) {
        print("Treewidth is too large for N=% variables.\n", N);
        std::exit(-1);
    }
    treewidth = config[MAX_CLIQUE].number == -1? N: config[MAX_CLIQUE].number;
    print("CSV file is read. Instance has % samples, N=%, ess=%\n",
        csv->size(), N, config[ESS].number);

    if(config[PRECOMP_LVL] != 0)
        print("============================ Precomputing clique scores\n");
    
    if(config[PRECOMP_LVL] == 1) {
        size_t tw = 2;
        if(tw > treewidth) tw = treewidth;
        for(;;) {
            size_t count = clique_count(N, tw);
            if(count >= 500000)
                break;
            size_t time = precompute_scores(tw);
            tw++;
            if(time >= 1000 || tw > treewidth)
                break;
        }
        return;
    }
    if(config[PRECOMP_LVL] > 0)
        precompute_scores(treewidth);
}

Instance::Instance(const Config& conf): config(conf) {
    if(config.filename().find(".csv") == config.filename().length() - 4)
        read_CSV_file();
    else
        read_score_file();
}

bool Instance::increase_dynamic_treewidth() {
    size_t max_tw = config[MAX_CLIQUE].number == -1? N: config[MAX_CLIQUE].number;
    if(treewidth >= max_tw)
        return false;
    
    if(treewidth < 4)
        treewidth++;
    else
        treewidth *= 2;
    if(treewidth > max_tw - max_tw/4)
        treewidth = max_tw;
    return true;
}

void Instance::reset_dynamic_treewidth() {
    size_t max = config[MAX_CLIQUE].number == -1? N: config[MAX_CLIQUE].number;
    treewidth = max < 3? max: 3;
}

std::ostream& operator<<(std::ostream& os, const Clique& clique) {
    bool any = false;
    for(size_t i = 0; i < clique.size(); i++) {
        if(!clique.test(i))
            continue;
        if(any) os << ",";
        any = true;
        os << i;
    }
    return os;
}
