#include <ICF.hpp>
#include <util.hpp>
#include <random>
#include <unordered_set>

using namespace Eigen;

struct Range {    
    node_t a, n;
    
    Range(node_t a, node_t b): a(a), n(b - a) {}
    
    inline node_t operator[](node_t i) const { return a + i; }
    inline node_t size() const { return n; }
};
using range = Range;

struct ExcludingRange {
    node_t x, n;
    
    ExcludingRange(node_t x, node_t n): x(x), n(n) {}
    
    inline node_t operator[](node_t i) const { return i < x? i: i + 1; }
    inline node_t size() const { return n - 1; }
};

inline MatrixXd invert(const MatrixXd& matrix) {
    return matrix.inverse();
}

inline VectorXd vectorize(MatrixXd matrix) {
    matrix.transposeInPlace();
    return Map<VectorXd>(matrix.data(), matrix.cols() * matrix.rows());
}

#define absolute(x) ((x) < 0? -(x): (x))

double sum_of_diff(const MatrixXd& m1, const MatrixXd& m2) {
    ensure(m1.rows() == m2.rows() && m1.cols() == m2.cols());
    
    double diff = 0;
    for(size_t i = 0; i < m1.size(); i++)
        diff += absolute(m1.data()[i] - m2.data()[i]);
    return diff;
}

void ICF::run_icf_iteration(MatrixXd& Omega, MatrixXd& B, const MatrixXd& S, const std::vector<Nodes>& parents, const std::vector<Nodes>& spouses) const {
    const node_t p = S.rows();
    
    #define exclude(var) ExcludingRange(var, p)
    
    for(size_t v = 0; v < spouses.size(); v++) {
        const Nodes& spov = spouses[v];
        const Nodes& parv = parents[v];

        ensure(!spov.empty());
        
        MatrixXd inverse = MatrixXd::Zero(p, p);
        inverse(exclude(v), exclude(v)) = invert(Omega(exclude(v), exclude(v)));

        MatrixXd Z = inverse(spov, exclude(v)) * B(exclude(v), all);

        if(!parv.empty()) {
            size_t lpa = parv.size();
            size_t lspo = spov.size();

            MatrixXd XX = MatrixXd::Zero(lpa+lspo, lpa+lspo);
            XX(range(0, lpa), range(0, lpa)) = S(parv, parv);
            XX(range(0, lpa), range(lpa, lpa+lspo)) = S(parv, all) * Z.transpose();
            XX(range(lpa, lpa+lspo), range(0, lpa)) = XX(range(0, lpa), range(lpa, lpa+lspo)).transpose();
            XX(range(lpa, lpa+lspo), range(lpa, lpa+lspo)) = Z * S * Z.transpose();

            VectorXd YX(lpa+lspo);
            YX(range(0, lpa)) = S(v, parv);
            YX(range(lpa, lpa+lspo)) = vectorize(S(v, all) * Z.transpose());

            MatrixXd temp = YX.transpose() * invert(XX);
            B(v, parv) = -temp(0, range(0, lpa));
            
            Omega(v, spov) = temp(0, range(lpa, lpa+lspo));
            Omega(spov, v) = Omega(v, spov);
            Omega(v, v) = S(v, v) - temp(0, all) * YX +
                Omega(v, spov) * inverse(spov, spov) * Omega(spov, v);

        } else {
            MatrixXd XX = Z * S * Z.transpose();
            VectorXd YX = vectorize(S(v, all) * Z.transpose());

            Omega(v, spov) = YX.transpose() * invert(XX);
            Omega(spov, v) = Omega(v, spov);
            Omega(v, v) = S(v, v) - Omega(v, spov) * YX +
                Omega(v, spov) * inverse(spov, spov) * Omega(spov, v);
        }
    }
}

bits_t get_bitset(const Nodes& labels, const Nodes& set) {
    bits_t bits;
    for(node_t item: set)
        bits.set(labels[item]);
    return bits;
}

struct Randomizer {
    Randomizer(): gen(get_time()) {}
    
    static Randomizer& global() {
        static Randomizer r;
        return r;
    }
    
    MatrixXd get_B(node_t size, const std::vector<Nodes>& parents) {
        MatrixXd B = MatrixXd::Zero(size, size);
        for(node_t i = 0; i < parents.size(); i++)
            for(node_t j: parents[i])
                B(i, j) = std::uniform_real_distribution<>(-3.0, 3.0)(gen);
        for(node_t v = 0; v < size; v++)
            B(v, v) = 1;
        return B;
    }

    MatrixXd get_Omega(node_t size, const std::vector<Nodes>& spouses, const MatrixXd& S) {
        MatrixXd Omega = MatrixXd::Zero(size, size);
        for(node_t i = 0; i < spouses.size(); i++)
            for(node_t j: spouses[i])
                if(i < j) {
                    Omega(i, j) = std::uniform_real_distribution<>(-3.0, 3.0)(gen);
                    Omega(j, i) = Omega(i, j);
                }
        for(node_t v = 0; v < spouses.size(); v++)
            Omega(v, v) = std::uniform_real_distribution<>(0.1, 10)(gen);
        for(node_t v = spouses.size(); v < size; v++)
            Omega(v, v) = S(v, v);
        return Omega;
    }
    
    std::mt19937 gen;
};

MatrixXd ICF::run_icf(const MatrixXd& S, const std::vector<Nodes>& parents, const std::vector<Nodes>& spouses, const Nodes& nodes) const {
    ensure(S.rows() == S.cols());
    size_t start_time = get_time();
    size_t iters = 0;

    MatrixXd B = MatrixXd::Identity(S.rows(), S.cols());
    MatrixXd Omega = MatrixXd::Zero(S.rows(), S.cols());
    Omega.diagonal() = S.diagonal();

    if(spouses.size() == 1) {
        if(!parents[0].empty()) {
            B(0, parents[0]) = -S(0, parents[0]) * invert(S(parents[0], parents[0]));
            Omega(0, 0) = S(0, 0) + B(0, parents[0]) * S(parents[0], 0);
        }
    } else {
        size_t max_iters_local = 100;
        restart:
        if(use_rand_start) {
            B = Randomizer::global().get_B(S.cols(), parents);
            Omega = Randomizer::global().get_Omega(S.cols(), spouses, S);
        }
        
        iters = 0;
        for(; iters < MaxIters; iters++) {
            MatrixXd old_B = B;
            MatrixXd old_Omega = Omega;

            run_icf_iteration(Omega, B, S, parents, spouses);

            if(sum_of_diff(Omega, old_Omega) + sum_of_diff(B, old_B) < tolerance)
                break;
            else if(use_rand_start && iters > max_iters_local) {
                max_iters_local *= 2;
                goto restart;
            }
        }
    }

    icf_time += get_time() - start_time;
    total_iters += iters;
    if(most_iters < iters)
        most_iters = iters;
    MatrixXd result = invert(B) * Omega * invert(B.transpose());
    return result;
}

double ICF::calc_likelihood(const MatrixXd& cov_orig, const MatrixXd& cov_hat, double outer_product, size_t comp_size) const {
    double l1 = comp_size * std::log(2 * (double)3.1415926535);
    double l2 = std::log(cov_hat.determinant()) - std::log(outer_product);
    double l3 = (invert(cov_hat) * cov_orig).trace() - (cov_orig.rows() - comp_size);

    return l1 + l2 + l3;
}

double ICF::likelihood(const Scoreable& target) const {
    MatrixXd cov_orig = cov(target.nodes, target.nodes);
    MatrixXd cov_hat = run_icf(cov_orig, target.parents, target.spouses, target.nodes);

    double outer_product = 1;
    for(node_t outer_parent: target.outer_parents)
        outer_product *= cov_hat(outer_parent, outer_parent); //cov_orig(outer_parent, outer_parent);

    return calc_likelihood(cov_orig, cov_hat, outer_product, target.component_size());
}

double ICF::penalty(const Scoreable& target) const {
    return std::log(sample_count) / 2 * (target.edge_count + target.component_size() * 2);
}

double ICF::score(const Scoreable& target) const {
    use_rand_start = false;
    scored_count++;
    return likelihood(target) * -(sample_count / 2) - penalty(target);
}

ICF::Stats ICF::score(const Scoreable& target, size_t reps, size_t max_optimas) const {
    if(target.spouses.size() == 1)
        reps = 1;
    
    Stats stats {Scoreable::MinScore, -Scoreable::MinScore, 0, 0};
    std::unordered_set<double> set;
    
    use_rand_start = false;
    for(size_t i = 0; i < reps; i++) {
        if(i > 0)
            use_rand_start = true;
        
        double s = likelihood(target) * -(sample_count / 2) - penalty(target);
        stats.highest = std::max(s, stats.highest);
        stats.lowest = std::min(s, stats.lowest);
        stats.average += s / double(reps);
        
        bool over_already = set.size() >= max_optimas;
        set.insert(int(s * 1000));
        if(over_already)
            break;
    }
    stats.local_optimas = set.size();
    scored_count++;
    return stats;
}

MatrixXd read_matrix(std::istream& source) {
    std::vector<std::vector<double> > data;
    while(!source.eof()) {
        std::string line;
        std::getline(source, line);
        if(line.length() < 2)
            continue;
        data.push_back(std::vector<double>());
        for(const std::string& part: split(line, ' ')) {
            double value;
            std::istringstream(part) >> value;
            data.back().push_back(value);
        }
    }
    if(data.size() <= 1)
        error("Not enough rows in the input.\n");

    MatrixXd matrix(data.size(), data[0].size());
    for(size_t r = 0; r < data.size(); r++) {
        if(data[r].size() != data[0].size())
            error("Non-uniform number of columns between rows in the input.\n");

        for(size_t c = 0; c < data[r].size(); c++)
            matrix(r, c) = data[r][c];
    }
    return matrix;
}

MatrixXd calc_covariances(const MatrixXd& data) {
    std::vector<double> means(data.cols());
    for(size_t c = 0; c < data.cols(); c++) {
        means[c] = 0;
        for(size_t r = 0; r < data.rows(); r++)
            means[c] += data(r, c);
        means[c] /= data.rows();
    }

    MatrixXd cov = MatrixXd::Zero(data.cols(), data.cols());
    for(size_t j = 0; j < cov.cols(); j++)
        for(size_t k = 0; k < cov.cols(); k++) {
            for(size_t i = 0; i < data.rows(); i++)
                cov(j, k) += (data(i, j) - means[j]) * (data(i, k) - means[k]);
            cov(j, k) /= data.rows();
        }
    return cov;
}

MatrixXd to_correlation(const MatrixXd& cov) {
    MatrixXd corr = MatrixXd::Zero(cov.cols(), cov.cols());
    for(size_t j = 0; j < cov.cols(); j++)
        for(size_t k = 0; k < cov.cols(); k++)
            corr(j, k) = cov(j, k) / (sqrt(cov(j, j)) * sqrt(cov(k, k)));
    return corr;
}

std::vector<node_t> choose_variables(const MatrixXd& cov, size_t max_vars) {
    std::vector<node_t> vars;
    for(node_t v = 0; v < cov.cols(); v++)
        vars.push_back(v);

    MatrixXd corr = to_correlation(cov);
    while(vars.size() > max_vars) {
        double worst_var = vars[0], worst_corr = corr.cols();
        for(node_t i: vars) {
            double corr_sum = 0;
            for(node_t j: vars)
                corr_sum += absolute(corr(i, j));
            if(worst_corr > corr_sum) {
                worst_corr = corr_sum;
                worst_var = i;
            }
        }
        for(auto it = vars.begin();; it++) {
            ensure(it != vars.end());
            if(*it == worst_var) {
                vars.erase(it);
                break;
            }
        }
    }
    print("Picked variables [%] now called [0..%]\n", to_string(vars), vars.size()-1);
    return vars;
}

MatrixXd ICF::read_data(std::istream& source, size_t max_vars) {
    MatrixXd data = read_matrix(source);
    sample_count = data.rows();
    
    MatrixXd full = calc_covariances(data);
    if(max_vars && max_vars < full.cols()) {
        std::vector<node_t> chosen = choose_variables(full, max_vars);
        cov = full(chosen, chosen);
    } else
        cov = full;
    return data;
}

Eigen::MatrixXd ICF::read_covariances(std::istream& source, size_t max_vars) {
    std::string line;
    getline(source, line);
    std::stringstream stream(line);
    stream >> sample_count;
    
    MatrixXd full = read_matrix(source);
    if(full.cols() != full.rows())
        error("Mismatching number of rows and columns in the given covariance matrix.\n");
    
    if(max_vars && max_vars < full.cols()) {
        std::vector<node_t> chosen = choose_variables(full, max_vars);
        cov = full(chosen, chosen);
    } else
        cov = full;
    return cov;
}
