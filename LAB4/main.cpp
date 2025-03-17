#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <queue>
#include <stack>
#include <algorithm>

using namespace std;

class Graph {
private:
    int vertices;
    vector<vector<int>> adj_matrix;
    vector<vector<int>> inc_matrix;
    vector<vector<int>> adj_list;
    vector<pair<int, int>> edge_list;
    bool is_directed;

public:
    Graph(int v, bool dir) : vertices(v), is_directed(dir) {
        adj_matrix.resize(v, vector<int>(v, 0));
        inc_matrix.resize(v, vector<int>(0));
        adj_list.resize(v);
    }

    [[nodiscard]] bool hasEdge(int from, int to) const {
        return adj_matrix[from][to] == 1;
    }

    void addEdge(int from, int to) {
        adj_matrix[from][to] = 1;
        if (!is_directed) adj_matrix[to][from] = 1;
        adj_list[from].push_back(to);
        if (!is_directed) adj_list[to].push_back(from);
        edge_list.emplace_back(from, to);

        for (int i = 0; i < vertices; ++i) {
            if (i == from) inc_matrix[i].push_back(1);
            else if (i == to) inc_matrix[i].push_back(is_directed ? -1 : 1);
            else inc_matrix[i].push_back(0);
        }
    }

    void printAdjacencyMatrix() {
        cout << "Adjacency Matrix:\n";
        for (int i = 0; i < vertices; i++) {
            for (int j = 0; j < vertices; j++) cout << adj_matrix[i][j] << " ";
            cout << "\n";
        }
    }

    void printIncidenceMatrix() {
        cout << "Incidence Matrix:\n";
        int num_edges = inc_matrix[0].size();
        for (int i = 0; i < vertices; i++) {
            for (int j = 0; j < num_edges; j++) cout << setw(3) << inc_matrix[i][j];
            cout << "\n";
        }
    }

    void printAdjacencyList() {
        cout << "Adjacency List:\n";
        for (int i = 0; i < vertices; i++) {
            cout << i << ": ";
            for (int j : adj_list[i]) cout << j << " ";
            cout << "\n";
        }
    }

    void printEdgeList() {
        cout << "Edge List:\n";
        for (auto& edge : edge_list) cout << "(" << edge.first << ", " << edge.second << ") ";
        cout << "\n";
    }

    [[nodiscard]] int getVertices() const { return vertices; }
    vector<int>& getAdjList(int v) { return adj_list[v]; }
    [[nodiscard]] int getEdgeCount() const { return edge_list.size(); } // Новый метод для получения числа рёбер
};

Graph generateRandomGraph(int v, int e, int max_edges_per_vertex, bool is_directed, int max_in_out_edges) {
    random_device rd;
    mt19937 gen(rd());
    Graph graph(v, is_directed);

    if (v <= 1) return graph;

    vector<int> out_degrees(v, 0);
    vector<int> in_degrees(v, 0);
    uniform_int_distribution<> vertex_choice(0, v - 1);

    int attempts = 0, max_attempts = 10000;
    int added_edges = 0;
    while (added_edges < e && attempts < max_attempts) {
        int from = vertex_choice(gen);
        int to = vertex_choice(gen);
        attempts++;

        if (from == to || graph.hasEdge(from, to) ||
            out_degrees[from] >= max_edges_per_vertex ||
            (is_directed && (out_degrees[from] >= max_in_out_edges || in_degrees[to] >= max_in_out_edges))) {
            continue;
        }

        graph.addEdge(from, to);
        out_degrees[from]++;
        if (is_directed) in_degrees[to]++;
        else out_degrees[to]++;
        added_edges++;
    }

    return graph;
}

int bfs(Graph& g, int start, int end, vector<int>& path) {
    int v_count = g.getVertices();
    vector<int> distance(v_count, -1);
    vector<int> parent(v_count, -1);
    queue<int> q;

    distance[start] = 0;
    q.push(start);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : g.getAdjList(u)) {
            if (distance[v] == -1) {
                distance[v] = distance[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
    }

    if (distance[end] == -1) return -1;
    int curr = end;
    while (curr != -1) {
        path.push_back(curr);
        curr = parent[curr];
    }
    if (!path.empty()) { // Проверка на пустоту
        reverse(path.begin(), path.end());
    }
    return distance[end];
}

int dfs(Graph& g, int start, int end, vector<int>& path) {
    int v_count = g.getVertices();
    vector<bool> visited(v_count, false);
    vector<int> parent(v_count, -1);
    stack<int> s;
    s.push(start);

    while (!s.empty()) {
        int u = s.top();
        s.pop();

        if (u == end) {
            visited[u] = true; // Помечаем конечную вершину как посещённую
            break;
        }

        if (!visited[u]) {
            visited[u] = true;
            // Добавляем соседей в обратном порядке для корректного порядка обработки
            vector<int> neighbors = g.getAdjList(u);
            reverse(neighbors.begin(), neighbors.end());
            for (int v : neighbors) {
                if (!visited[v]) {
                    parent[v] = u;
                    s.push(v);
                }
            }
        }
    }

    if (!visited[end]) return -1;
    int curr = end;
    while (curr != -1) {
        path.push_back(curr);
        curr = parent[curr];
    }
    if (!path.empty()) { // Проверка на пустоту
        reverse(path.begin(), path.end());
    }
    return path.size() - 1;
}

void generateGraphData(const vector<double>& bfs_times, const vector<double>& dfs_times, const vector<int>& sizes, const vector<bool>& directed, const vector<int>& edges) {
    ofstream file("graph_data.csv");
    if (file.is_open()) {
        file << "Graph,Size,Edges,Directed,BFS_Time,DFS_Time\n"; // Добавлен столбец Edges
        for (size_t i = 0; i < sizes.size(); ++i) {
            file << i + 1 << "," << sizes[i] << "," << edges[i] << "," << (directed[i] ? "Yes" : "No") << "," << bfs_times[i] << "," << dfs_times[i] << "\n";
        }
        file.close();
        cout << "Graph data saved to graph_data.csv. Use a plotting tool (e.g., Python) to visualize.\n";
    }
}

int main() {
    random_device rd;
    mt19937 gen(rd());

    int min_vertices = 1000, max_vertices = 10000;
    int min_edges = 5000, max_edges = 50000;
    int step_vertices = (max_vertices - min_vertices) / 9;
    int step_edges = (max_edges - min_edges) / 9;
    int max_edges_per_vertex = 5;
    int max_in_out_edges = 3;

    vector<int> sizes;
    vector<bool> directed_flags;
    vector<Graph> graphs;
    vector<double> bfs_times, dfs_times;
    vector<int> edge_counts; // Новый вектор для хранения количества рёбер

    // Генерация 10 направленных графов
    cout << "Generating Directed Graphs:\n";
    for (int i = 0; i < 10; ++i) {
        int v = min_vertices + i * step_vertices;
        int e = min_edges + i * step_edges;
        e = min(e, v * (v - 1));
        sizes.push_back(v);
        directed_flags.push_back(true);

        Graph g = generateRandomGraph(v, e, max_edges_per_vertex, true, max_in_out_edges);
        graphs.push_back(g);
        edge_counts.push_back(g.getEdgeCount()); // Сохраняем количество рёбер

        uniform_int_distribution<> vertex_choice(0, v - 1);
        int start = vertex_choice(gen);
        int end = vertex_choice(gen);
        while (start == end) end = vertex_choice(gen);

        vector<int> bfs_path;
        auto bfs_start = chrono::high_resolution_clock::now();
        int bfs_dist = bfs(g, start, end, bfs_path);
        auto bfs_end = chrono::high_resolution_clock::now();
        double bfs_time = chrono::duration<double>(bfs_end - bfs_start).count();
        bfs_times.push_back(bfs_time);
        cout << "Directed Graph " << i + 1 << " (V=" << v << ", E=" << g.getEdgeCount() << "):\n";
        cout << "BFS from " << start << " to " << end << ": ";
        if (bfs_dist == -1) cout << "No path\n";
        else {
            cout << "Shortest Distance = " << bfs_dist << ", Path = ";
            for (int vertex : bfs_path) cout << vertex << " ";
        }
        cout << "\nTime: " << bfs_time << "s\n";

        vector<int> dfs_path;
        auto dfs_start = chrono::high_resolution_clock::now();
        int dfs_dist = dfs(g, start, end, dfs_path);
        auto dfs_end = chrono::high_resolution_clock::now();
        double dfs_time = chrono::duration<double>(dfs_end - dfs_start).count();
        dfs_times.push_back(dfs_time);
        cout << "DFS from " << start << " to " << end << ": ";
        if (dfs_dist == -1) cout << "No path\n";
        else {
            cout << "Path Length = " << dfs_dist << ", Path = ";
            for (int vertex : dfs_path) cout << vertex << " ";
        }
        cout << "\nTime: " << dfs_time << "s\n\n";

        if (i == 0) {
            g.printAdjacencyMatrix();
            g.printIncidenceMatrix();
            g.printAdjacencyList();
            g.printEdgeList();
        }
    }

    // Генерация 10 ненаправленных графов
    cout << "Generating Undirected Graphs:\n";
    for (int i = 0; i < 10; ++i) {
        int v = min_vertices + i * step_vertices;
        int e = min_edges + i * step_edges;
        e = min(e, v * (v - 1) / 2);
        sizes.push_back(v);
        directed_flags.push_back(false);

        Graph g = generateRandomGraph(v, e, max_edges_per_vertex, false, max_in_out_edges);
        graphs.push_back(g);
        edge_counts.push_back(g.getEdgeCount()); // Сохраняем количество рёбер

        uniform_int_distribution<> vertex_choice(0, v - 1);
        int start = vertex_choice(gen);
        int end = vertex_choice(gen);
        while (start == end) end = vertex_choice(gen);

        vector<int> bfs_path;
        auto bfs_start = chrono::high_resolution_clock::now();
        int bfs_dist = bfs(g, start, end, bfs_path);
        auto bfs_end = chrono::high_resolution_clock::now();
        double bfs_time = chrono::duration<double>(bfs_end - bfs_start).count();
        bfs_times.push_back(bfs_time);
        cout << "Undirected Graph " << i + 1 << " (V=" << v << ", E=" << g.getEdgeCount() << "):\n";
        cout << "BFS from " << start << " to " << end << ": ";
        if (bfs_dist == -1) cout << "No path\n";
        else {
            cout << "Shortest Distance = " << bfs_dist << ", Path = ";
            for (int vertex : bfs_path) cout << vertex << " ";
        }
        cout << "\nTime: " << bfs_time << "s\n";

        vector<int> dfs_path;
        auto dfs_start = chrono::high_resolution_clock::now();
        int dfs_dist = dfs(g, start, end, dfs_path);
        auto dfs_end = chrono::high_resolution_clock::now();
        double dfs_time = chrono::duration<double>(dfs_end - dfs_start).count();
        dfs_times.push_back(dfs_time);
        cout << "DFS from " << start << " to " << end << ": ";
        if (dfs_dist == -1) cout << "No path\n";
        else {
            cout << "Path Length = " << dfs_dist << ", Path = ";
            for (int vertex : dfs_path) cout << vertex << " ";
        }
        cout << "\nTime: " << dfs_time << "s\n\n";

        if (i == 0) {
            g.printAdjacencyMatrix();
            g.printIncidenceMatrix();
            g.printAdjacencyList();
            g.printEdgeList();
        }
    }

    generateGraphData(bfs_times, dfs_times, sizes, directed_flags, edge_counts);

    double avg_bfs_directed = 0, avg_dfs_directed = 0;
    double avg_bfs_undirected = 0, avg_dfs_undirected = 0;
    for (size_t i = 0; i < bfs_times.size(); ++i) {
        if (directed_flags[i]) {
            avg_bfs_directed += bfs_times[i];
            avg_dfs_directed += dfs_times[i];
        } else {
            avg_bfs_undirected += bfs_times[i];
            avg_dfs_undirected += dfs_times[i];
        }
    }
    avg_bfs_directed /= 10;
    avg_dfs_directed /= 10;
    avg_bfs_undirected /= 10;
    avg_dfs_undirected /= 10;

    cout << "Analysis:\n";
    cout << "Directed Graphs:\n";
    cout << "Average BFS time: " << avg_bfs_directed << "s\n";
    cout << "Average DFS time: " << avg_dfs_directed << "s\n";
    cout << "Undirected Graphs:\n";
    cout << "Average BFS time: " << avg_bfs_undirected << "s\n";
    cout << "Average DFS time: " << avg_dfs_undirected << "s\n";
    cout << "BFS finds the shortest path efficiently, while DFS may fail if vertices are in different components.\n";

    return 0;
}