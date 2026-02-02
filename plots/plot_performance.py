import matplotlib.pyplot as plt
import numpy as np

# ===============================================================
# Input Section:
# Enter the execution times obtained from the C++ console output
# (All values are in microseconds)
# ===============================================================

# 1. Alpha = 0.15
time_ppr_15 = 10978     # Power Iteration execution time
time_mc_15  = 2167591  # Monte Carlo execution time

# 2. Alpha = 0.50
time_ppr_50 = 7151
time_mc_50  = 1066008

# 3. Alpha = 0.85
time_ppr_85 = 3431
time_mc_85  = 435158

# ===============================================================

# Prepare data for plotting
labels = ['Alpha = 0.15', 'Alpha = 0.50', 'Alpha = 0.85']
ppr_times = [time_ppr_15, time_ppr_50, time_ppr_85]
mc_times = [time_mc_15, time_mc_50, time_mc_85]

x = np.arange(len(labels))   # X-axis positions
width = 0.35                 # Width of each bar

# Create figure and axis
fig, ax = plt.subplots(figsize=(10, 6))

# Draw bar charts
rects1 = ax.bar(
    x - width / 2,
    ppr_times,
    width,
    label='Power Iteration (Exact)',
    color='#4CAF50'
)

rects2 = ax.bar(
    x + width / 2,
    mc_times,
    width,
    label='Monte Carlo (Approximate)',
    color='#FF9800'
)

# Chart formatting
ax.set_ylabel('Execution Time (Microseconds)')
ax.set_title('Performance Comparison: Exact vs. Approximate Methods')
ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()
ax.grid(axis='y', linestyle='--', alpha=0.3)

# Attach execution time labels on top of each bar
def autolabel(rects):
    for rect in rects:
        height = rect.get_height()
        ax.annotate(
            f'{height}',
            xy=(rect.get_x() + rect.get_width() / 2, height),
            xytext=(0, 3),  # Vertical offset
            textcoords="offset points",
            ha='center',
            va='bottom',
            fontweight='bold',
            fontsize=9
        )

autolabel(rects1)
autolabel(rects2)

# Adjust layout and save figure
fig.tight_layout()
plt.savefig('plot_performance_comparison.png')
plt.show()
