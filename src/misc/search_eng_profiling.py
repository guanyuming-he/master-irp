"""
@author Guanyuming He
Copyright (C) Guanyuming He 2025
The file is licensed under the GNU GPL v3.

The file profiles the search engine response time.
"""

import sys
import subprocess
import time
import statistics
import matplotlib.pyplot as plt
import numpy as np

# Configuration
SEARCH_CMD = ["./bin/searcher", "./db"]
REPEATS = 5

def run_query(query: str) -> float:
	start = time.perf_counter()
	# Don't need their output
	subprocess.run(
		SEARCH_CMD + [query], 
		stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
	)
	end = time.perf_counter()
	return end - start

def main():
	queries = [line.strip() for line in sys.stdin if line.strip()]
	if not queries:
		print("No queries provided on stdin.", file=sys.stderr)
		sys.exit(1)

	print(f"num of queries = {len(queries)}")

	results = []
	for query in queries:
		times = [run_query(query) for _ in range(REPEATS)]
		avg = statistics.mean(times)
		stdev = statistics.stdev(times) if len(times) > 1 else 0.0
		print(f"avg={avg}, stdev={stdev}")
		results.append((query, avg, stdev))

	# Sort by avg time descending
	results.sort(key=lambda x: x[1], reverse=True)

	# Print table
	print(f"{'Query':60} {'Avg Time (s)':>12} {'StdDev (s)':>12}")
	print("-" * 86)
	for query, avg, stdev in results:
		print(f"{query:60} {avg:12.6f} {stdev:12.6f}")

	# Prepare data for plotting
	query_lengths = [len(q.split()) for q, _, _ in results]
	avg_times = [avg for _, avg, _ in results]

	# Get unique query lengths and their average times (grouped)
	unique_lengths = sorted(set(query_lengths))
	avg_time_per_length = []
	for length in unique_lengths:
		times_for_length = [
			avg for q_len, avg in zip(query_lengths, avg_times) 
			if q_len == length
		]
		avg_time_per_length.append(np.mean(times_for_length))

	plt.figure(figsize=(10, 6))
	plt.scatter(
		query_lengths, avg_times, alpha=0.3, label="Individual queries"
	)
	plt.plot(
		unique_lengths, avg_time_per_length, 
		'r-o', label="Average per query length", linewidth=2
	)

	plt.title("Average Search Response Time vs Query Length")
	plt.xlabel("Query Length (number of words)")
	plt.ylabel("Average Response Time (seconds)")
	plt.xticks(unique_lengths)  # only show integer ticks that exist in data
	plt.xlim(min(unique_lengths) - 0.5, max(unique_lengths) + 0.5)
	plt.grid(True)
	plt.legend()
	plt.savefig("avg_time_vs_query_length.png")
	print("\nPlot saved to avg_time_vs_query_length.png")

if __name__ == "__main__":
	main()
