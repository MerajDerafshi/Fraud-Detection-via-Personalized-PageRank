# Fraud Detection via Personalized PageRank (PPR)

A graph-based fraud detection system using **Personalized PageRank (PPR)** to propagate suspicion scores from known fraudsters to the rest of the network.

---

## 👤 Authors

- **[Sama Esmaelifar](https://github.com/sama205)**
- **[Meraj Derafshi](https://github.com/MerajDerafshi)**

**Instructor:** Dr. Katanforoush  
**Course:** Data Structures  
**Semester:** Fall 2025

---

## 📌 Problem Overview

Fraudulent activities in financial and social networks tend to form **connected clusters** rather than isolated events.  
This project is based on the **Guilt by Association** principle: nodes that are strongly connected to known fraudsters are more likely to be fraudulent.

### Goals

- Model transactions as a **directed graph**
- Use **Personalized PageRank (PPR)** to propagate suspicion scores
- Automatically identify **high-risk nodes**

---

## 🧠 Algorithms Implemented

### 1️⃣ Personalized PageRank (Exact)

- Power Iteration method
- CSR (Compressed Sparse Row) graph representation
- Dead-end (dangling node) handling
- Convergence based on **L1 norm**

### 2️⃣ Monte Carlo Approximation (Bonus)

- Random walk simulation
- Weighted neighbor selection
- Faster execution with approximate results

---

## 🗂️ Dataset Format

The system supports **both weighted and unweighted directed graphs**.

### ✅ Unweighted Graph (2 Columns)

```
nodeA nodeB
nodeB nodeC
nodeC nodeA
```

- Interpreted as: `nodeA → nodeB`
- Default edge weight = **1.0**

---

### ✅ Weighted Graph (3 Columns)

```
nodeA nodeB 5.2
nodeB nodeC 1.0
nodeC nodeA 0.3
```

- Third column represents the **edge weight**
- Weights are automatically **normalized**
- Negative weights are converted to their **absolute values**

---

### 📝 Notes

- Lines starting with `#` or `%` are ignored
- Blank lines are allowed
- Graph is **directed**
- If no weights are provided, the graph is treated as **unweighted**

---

## 🧱 Graph Representation

The graph is stored using **Compressed Sparse Row (CSR)** format:

- `row_ptr`
- `col_indices`
- `edge_weights`
- `out_weight_sum`

This reduces memory complexity from **O(N²)** to **O(N + E)**.

---

## ⚙️ Configuration Options

- Interactive seed (known fraudster) selection
- Dynamic Monte Carlo walks:

```
total_walks = number_of_nodes × 500
```

- Multiple damping factors tested:

```
α ∈ {0.15, 0.50, 0.85}
```

---

## 📊 Outputs

The program generates **CSV reports** for each configuration:

- `results_PPR_alpha_15.csv`
- `results_PPR_alpha_50.csv`
- `results_PPR_alpha_85.csv`
- `results_MC_alpha_15.csv`
- `results_MC_alpha_50.csv`
- `results_MC_alpha_85.csv`

Each CSV file contains:

| Column | Description |
|------|------------|
| Rank | Node ranking based on score |
| NodeID | Unique node identifier |
| Score | Fraud suspicion score |
| Status | Fraud / Normal |

---

## 🚀 How to Run

Compile and execute:

```bash
g++ -O2 main.cpp -o fraud_detection
./fraud_detection
```

Then enter the dataset filename when prompted:

```
Enter dataset filename: facebook_combined.txt
```

---

## ⚠️ Limitations

- Cold-start problem for newly added nodes
- Assumes a **static graph** (no temporal updates)
- Requires reliable **seed (known fraudster) selection**

---

## 📚 References

- Page, L. et al. *The PageRank Citation Ranking*
- Gyöngyi, Z. et al. *Combating Web Spam with TrustRank*
- Stanford SNAP Datasets  
  http://snap.stanford.edu/data/

