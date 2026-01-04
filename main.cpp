#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <omp.h>

using namespace std;

class CSRMatrix {
public:
    vector<double> values;
    vector<int> colIndex;
    vector<int> rowIndex;
    int numRows;

    CSRMatrix(int rows, int cols) : numRows(rows) {
        rowIndex.resize(rows + 1, 0);
    }

    void addValue(int row, int col, double value) {
        values.push_back(value);
        colIndex.push_back(col);
        rowIndex[row + 1]++;
    }

    void finalizeRowIndex() {
        for (int i = 1; i <= numRows; ++i) {
            rowIndex[i] += rowIndex[i - 1];
        }
    }

    vector<double> multiply(const vector<double>& vec) {
        int n = rowIndex.size() - 1;
        vector<double> result(n, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int j = rowIndex[i]; j < rowIndex[i + 1]; ++j) {
                result[i] += values[j] * vec[colIndex[j]];
            }
        }
        return result;
    }

    void normalizeWeights() {
        for (int i = 0; i < numRows; ++i) {
            double outDegree = rowIndex[i + 1] - rowIndex[i];
            if (outDegree > 0) {
                for (int j = rowIndex[i]; j < rowIndex[i + 1]; ++j) {
                    values[j] /= outDegree;
                }
            }
        }
    }

    void handleDeadEnds(vector<double>& p, double alpha) {
        for (int i = 0; i < numRows; ++i) {
            if (rowIndex[i] == rowIndex[i + 1]) {
                p[i] += alpha / numRows;
            }
        }
    }

    void readFromFile(const string& filename) {
        ifstream file(filename);
        int from, to;
        double weight;
        vector<pair<int, pair<int, double>>> edges;

        while (file >> from >> to >> weight) {
            edges.push_back({from, {to, weight}});
        }

        sort(edges.begin(), edges.end(), [](const pair<int, pair<int, double>>& a, const pair<int, pair<int, double>>& b) {
            return a.first < b.first;
        });

        for (const auto& edge : edges) {
            addValue(edge.first, edge.second.first, edge.second.second);
        }

        finalizeRowIndex();
    }
};

vector<double> personalizedPageRank(CSRMatrix& M, const vector<double>& p, double alpha = 0.15, double epsilon = 1e-6, int max_iter = 100) {
    int n = p.size();
    vector<double> r(n, 0.0);

    for (int i = 0; i < n; ++i) {
        if (p[i] > 0) r[i] = p[i];
    }

    for (int iter = 0; iter < max_iter; ++iter) {
        vector<double> r_new = M.multiply(r);
        
        for (int i = 0; i < n; ++i) {
            r_new[i] = (1 - alpha) * r_new[i] + alpha * p[i];
        }

        M.handleDeadEnds(r_new, alpha);

        double norm = 0.0;
        #pragma omp parallel for reduction(+:norm)
        for (int i = 0; i < n; ++i) {
            norm += fabs(r_new[i] - r[i]);
        }

        if (norm < epsilon) {
            cout << "Converged in " << iter + 1 << " iterations." << endl;
            return r_new;
        }

        r = r_new;
    }

    cout << "Max iterations reached." << endl;
    return r;
}

int main() {
    int n = 5;
    CSRMatrix M(n, n);
    
    M.readFromFile("transactions.txt");

    M.normalizeWeights();

    vector<int> trustedNodes = {0, 2};
    vector<double> p(n, 0.0);
    for (int nodeID : trustedNodes) {
        p[nodeID] = 1.0 / trustedNodes.size();
    }

    vector<double> result = personalizedPageRank(M, p);

    cout << "\n--- Fraud Detection Results ---\n";
    for (int i = 0; i < n; ++i) {
        if (result[i] < 0.001) {
            cout << "Node " << i << " is SUSPICIOUS (Low Trust Rank: " << result[i] << ")" << endl;
        }
    }

    return 0;
}
