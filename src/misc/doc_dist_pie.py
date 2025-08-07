"""
@author Guanyuming He
Copyright (C) Guanyuming He 2025
The file is licensed under the GNU GPL v3.

The file turns output from 
src/search/tools/doc_dist.cpp
into a pie chart.
"""

import sys
import matplotlib.pyplot as plt
import re

if __name__ == "__main__":
	input_data = sys.stdin.read()

	# Extract total document count
	total_match = re.search(r'Num total doc\s*=\s*(\d+)', input_data)
	total_docs = int(total_match.group(1)) if total_match else 1

	# Host: <host> <count> (<percentage>)
	pattern = r'Host:\s*([^:]+):\s*(\d+)\s*\(([\d.]+)%\)'
	matches = re.findall(pattern, input_data)

	hosts = []
	counts = []

	for host, count, _ in matches:
		hosts.append(host)
		counts.append(int(count))

	# Build labels for large slices and collect legend entries for small slices
	threshold = 5
	labels = []
	legend_entries = []

	# If the percentage >= 5.
	# then, show it on labels.
	# Otherwise, show it on legends.
	for host, count in zip(hosts, counts):
		pct = count / total_docs * 100
		if pct >= threshold:
			labels.append(f"{host}\n({pct:.1f}%)")
			legend_entries.append("")  
		else:
			labels.append("")
			legend_entries.append(f"{host} ({pct:.2f}%)")

	# Plot
	fig, ax = plt.subplots(figsize=(12, 12))
	wedges, texts, autotexts = ax.pie(
		counts,
		labels=labels,
		startangle=90,
		autopct=lambda p: f'{p:.1f}%' if p >= threshold else '',
		wedgeprops={'edgecolor': 'white'},
		textprops={'fontsize': 12}
	)

	# Add legend for small slices
	# Only include entries from legend_entries that are not empty
	legend_items = [w for w, label in zip(wedges, legend_entries) if label]
	legend_labels = [label for label in legend_entries if label]
	ax.legend(
		legend_items, legend_labels, 
		title=f"Minor Hosts (<{threshold}%)", loc="best", 
		bbox_to_anchor=(1.05, 0.5), fontsize=10
	)

	ax.set_title("Document Distribution by Host", fontsize=16)
	plt.tight_layout()
	plt.show()
