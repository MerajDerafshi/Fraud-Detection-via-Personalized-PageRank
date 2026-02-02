import pandas as pd
import matplotlib.pyplot as plt

# ===============================================================
# Step 1: Load the main result file (standard configuration)
# ===============================================================
try:
    df = pd.read_csv("results_PPR_alpha_15.csv")
except:
    print("CSV file not found.")
    exit()

# ===============================================================
# Step 2: Select top 20 nodes with highest suspicion scores
# ===============================================================
top_20 = df.head(20)

# Reverse order for better visualization in horizontal bar chart
top_20 = top_20.iloc[::-1]

# ===============================================================
# Step 3: Assign colors based on node status
# ===============================================================
colors = []
for status in top_20['Status']:
    if 'Seed' in status:
        colors.append('blue')       # Known fraudster (seed node)
    elif 'Suspicious' in status:
        colors.append('orange')     # High-risk / suspicious user
    else:
        colors.append('gray')       # Low-risk user

# ===============================================================
# Step 4: Plot horizontal bar chart of top suspects
# ===============================================================
plt.figure(figsize=(10, 8))
plt.barh(top_20['NodeID'], top_20['Score'], color=colors)

plt.xlabel('Fraud Score')
plt.title('Top 20 Most Suspicious Users (Fraud Detection Results)')

# ===============================================================
# Legend definition
# ===============================================================
from matplotlib.patches import Patch
legend_elements = [
    Patch(facecolor='blue', label='Seed (Known Fraudster)'),
    Patch(facecolor='orange', label='High Risk / Suspicious')
]
plt.legend(handles=legend_elements)

# Save and display the plot
plt.tight_layout()
plt.savefig('plot_top_suspects.png')
plt.show()
