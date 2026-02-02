#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

// =========================================================
// SECTION 1: Core Data Structures
// =========================================================

// Maps string-based node identifiers to compact integer IDs
// This reduces memory usage and speeds up graph processing
class NodeMapper {
    unordered_map<string, int> name_to_id;
    vector<string> id_to_name;

public:
    // Returns the ID of a node, creating it if it does not exist
    int getId(const string& name) {
        if (name_to_id.find(name) == name_to_id.end()) {
            int new_id = id_to_name.size();
            name_to_id[name] = new_id;
            id_to_name.push_back(name);
            return new_id;
        }
        return name_to_id[name];
    }

    // Converts numeric ID back to original node name
    string getName(int id) const {
        return (id >= 0 && id < id_to_name.size()) ? id_to_name[id] : "UNKNOWN";
    }

    int getNumNodes() const { return id_to_name.size(); }

    // Utility function: selects a random node name (used for auto seed selection)
    string getRandomNodeName() {
        if (id_to_name.empty()) return "";
        return id_to_name[rand() % id_to_name.size()];
    }
};

// Compressed Sparse Row (CSR) representation for directed weighted graphs
struct CSRGraph {
    int num_nodes;
    int num_edges;

    vector<int> row_ptr;          // Start index of outgoing edges per node
    vector<int> col_indices;      // Destination node IDs
    vector<double> edge_weights;  // Edge weights
    vector<double> out_weight_sum;// Sum of outgoing weights per node

    CSRGraph(int n) : num_nodes(n), num_edges(0) {
        row_ptr.resize(n + 1, 0);
        out_weight_sum.resize(n, 0.0);
    }
};

// =========================================================
// Graph Loader (Supports Weighted & Unweighted Datasets)
// =========================================================

CSRGraph loadGraphFromFile(const string& filename, NodeMapper& mapper) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: File '" << filename << "' not found!" << endl;
        exit(1);
    }

    struct Edge { int u, v; double w; };
    vector<Edge> temp_edges;
    string line;

    cout << "[Loader] Reading dataset..." << endl;

    // Read dataset line-by-line (robust to comments and blank lines)
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '%') continue;

        stringstream ss(line);
        string u_str, v_str;
        double weight = 1.0; // Default weight for unweighted graphs

        if (ss >> u_str >> v_str) {
            // Optional third column: edge weight
            double w_in;
            if (ss >> w_in) weight = fabs(w_in);

            // Prevent zero-weight edges (numerical stability)
            if (weight == 0) weight = 0.0001;

            int u = mapper.getId(u_str);
            int v = mapper.getId(v_str);
            temp_edges.push_back({u, v, weight});
        }
    }
    file.close();

    int N = mapper.getNumNodes();
    vector<vector<pair<int, double>>> adj(N);

    // Build adjacency list
    for (const auto& e : temp_edges)
        adj[e.u].push_back({e.v, e.w});

    // Convert adjacency list to CSR format
    CSRGraph graph(N);
    graph.num_edges = temp_edges.size();
    int cursor = 0;

    for (int i = 0; i < N; ++i) {
        graph.row_ptr[i] = cursor;
        double sum_w = 0.0;

        for (auto& nbr : adj[i]) {
            graph.col_indices.push_back(nbr.first);
            graph.edge_weights.push_back(nbr.second);
            sum_w += nbr.second;
            cursor++;
        }
        graph.out_weight_sum[i] = sum_w;
    }
    graph.row_ptr[N] = cursor;

    return graph;
}

// =========================================================
// SECTION 2: Algorithms
// =========================================================

struct AlgorithmResult {
    vector<double> scores;     // Final suspicion scores
    long long duration_us;     // Execution time
    int iterations;            // Iteration count / walks
};

// ---------- Personalized PageRank (Exact / Power Iteration) ----------

class PPREngine {
public:
    static AlgorithmResult compute(const CSRGraph& graph,
                                   const vector<int>& seeds,
                                   double alpha,
                                   double epsilon) {
        auto start = high_resolution_clock::now();
        int N = graph.num_nodes;

        // Personalization vector (probability mass on seed nodes)
        vector<double> p(N, 0.0);
        if (!seeds.empty()) {
            double mass = 1.0 / seeds.size();
            for (int id : seeds) if (id < N) p[id] = mass;
        }

        vector<double> r = p, r_new(N);
        int iter_count = 0;

        // Power Iteration loop
        for (int iter = 0; iter < 100; ++iter) {
            fill(r_new.begin(), r_new.end(), 0.0);
            double dead_mass = 0.0;

            // Push scores to outgoing neighbors
            for (int u = 0; u < N; ++u) {
                if (graph.out_weight_sum[u] > 0) {
                    for (int k = graph.row_ptr[u]; k < graph.row_ptr[u+1]; ++k) {
                        int v = graph.col_indices[k];
                        double w = graph.edge_weights[k];
                        r_new[v] += r[u] * (w / graph.out_weight_sum[u]);
                    }
                } else {
                    // Handle dead-end nodes
                    dead_mass += r[u];
                }
            }

            // Teleportation and convergence check
            double diff = 0.0;
            for (int i = 0; i < N; ++i) {
                double val = (1.0 - alpha) * r_new[i]
                           + alpha * p[i]
                           + (1.0 - alpha) * dead_mass * p[i];
                diff += fabs(val - r[i]);
                r_new[i] = val;
            }

            r = r_new;
            iter_count = iter + 1;
            if (diff < epsilon) break;
        }

        auto end = high_resolution_clock::now();
        return {r, duration_cast<microseconds>(end - start).count(), iter_count};
    }
};

// ---------- Monte Carlo Approximation (Bonus Method) ----------

class MonteCarloEngine {
public:
    static AlgorithmResult compute(const CSRGraph& graph,
                                   const vector<int>& seeds,
                                   double alpha,
                                   int total_walks) {
        auto start = high_resolution_clock::now();
        int N = graph.num_nodes;
        vector<int> visits(N, 0);

        if (seeds.empty()) return {vector<double>(N, 0.0), 0, 0};

        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> prob(0.0, 1.0);
        uniform_int_distribution<> seed_dist(0, seeds.size() - 1);

        // Random walk simulation
        for (int i = 0; i < total_walks; ++i) {
            int curr = seeds[seed_dist(gen)];

            while (true) {
                visits[curr]++;

                // Teleport / stop condition
                if (prob(gen) < alpha) break;
                if (graph.out_weight_sum[curr] == 0) break;

                // Weighted neighbor selection
                double target = prob(gen) * graph.out_weight_sum[curr];
                double acc = 0.0;

                for (int k = graph.row_ptr[curr]; k < graph.row_ptr[curr+1]; ++k) {
                    acc += graph.edge_weights[k];
                    if (target <= acc) {
                        curr = graph.col_indices[k];
                        break;
                    }
                }
            }
        }

        // Normalize visit counts to probabilities
        vector<double> scores(N, 0.0);
        long long total = 0;
        for (int v : visits) total += v;
        if (total > 0)
            for (int i = 0; i < N; ++i)
                scores[i] = (double)visits[i] / total;

        auto end = high_resolution_clock::now();
        return {scores, duration_cast<microseconds>(end - start).count(), total_walks};
    }
};

// =========================================================
// Utility: Save Results to CSV
// =========================================================

void saveToCSV(const string& filename,
               const vector<double>& scores,
               const NodeMapper& mapper,
               const vector<int>& seeds) {

    ofstream file(filename);
    file << "Rank,NodeID,Score,Status\n";

    vector<pair<double, int>> ranked;
    for (size_t i = 0; i < scores.size(); ++i)
        ranked.push_back({scores[i], i});

    sort(ranked.rbegin(), ranked.rend());

    for (size_t i = 0; i < ranked.size(); ++i) {
        int id = ranked[i].second;
        bool is_seed = find(seeds.begin(), seeds.end(), id) != seeds.end();

        string status = is_seed ? "Seed"
                        : (ranked[i].first > 0.0001 ? "Suspicious" : "Safe");

        file << (i+1) << "," << mapper.getName(id)
             << "," << ranked[i].first << "," << status << "\n";
    }

    file.close();
    cout << "-> Saved results to: " << filename << endl;
}

// =========================================================
// MAIN
// =========================================================

int main() {
    srand(time(0));
    cout << "=== FRAUD DETECTION SYSTEM (FINAL VERSION) ===\n";

    string filename;
    cout << "Enter dataset filename: ";
    cin >> filename;

    NodeMapper mapper;
    CSRGraph graph = loadGraphFromFile(filename, mapper);

    cout << "[Graph] Nodes: " << graph.num_nodes
         << " | Edges: " << graph.num_edges << endl;

    // Interactive seed selection
    vector<int> seed_ids;
    string input;
    cout << "\nEnter seed node names (type 'done' or 'random'):\n";
    while (true) {
        cout << "> ";
        cin >> input;
        if (input == "done") break;
        if (input == "random") {
            string rnd = mapper.getRandomNodeName();
            cout << "Auto-selected seed: " << rnd << endl;
            seed_ids.push_back(mapper.getId(rnd));
            break;
        }
        seed_ids.push_back(mapper.getId(input));
    }

    if (seed_ids.empty()) {
        cout << "No seeds selected. Exiting.\n";
        return 0;
    }

    long long dynamic_walks = graph.num_nodes * 500;
    vector<double> alpha_values = {0.15, 0.50, 0.85};

    for (double alpha : alpha_values) {
        string suffix = "_" + to_string((int)(alpha * 100)) + ".csv";

        auto res_ppr = PPREngine::compute(graph, seed_ids, alpha, 1e-6);
        saveToCSV("results_PPR_alpha" + suffix, res_ppr.scores, mapper, seed_ids);

        auto res_mc = MonteCarloEngine::compute(graph, seed_ids, alpha, dynamic_walks);
        saveToCSV("results_MC_alpha" + suffix, res_mc.scores, mapper, seed_ids);
    }

    cout << "\n[Done] All experiments completed successfully.\n";
    return 0;
}
