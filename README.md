# Gomory-Hu Tree

A C++ implementation of the **Gomory-Hu Tree**, a compact representation that allows efficient computation of **minimum cuts between all pairs of vertices** in an undirected, weighted graph.
The Gomory-Hu Tree is a weighted tree that represents all-pairs minimum cuts of a given undirected, weighted graph. For any pair of nodes `(u, v)`, the minimum cut capacity between them in the original graph is equal to the minimum edge weight along the unique path between `u` and `v` in the Gomory-Hu Tree.
It is useful in network reliability, clustering, flow analysis, and connectivity-related queries.

The algorithm works by:
1. Recursively partitioning the graph using **max-flow/min-cut** computations (e.g., Dinic’s algorithm).
2. Building a tree structure where each edge stores the min-cut capacity between the corresponding partitions.
3. Resulting tree can be used to answer any min-cut query in **O(log N)** time if LCA structures are added.

Time complexity: **O(V * MaxFlow)**, where `MaxFlow` is the time complexity of the underlying max-flow algorithm used.



