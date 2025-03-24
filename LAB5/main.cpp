#include <iostream>
#include <vector>
#include <random>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <climits>
#include <fstream>

using namespace std;

// Класс для работы с графом, где есть вершины и ребра с весами
class WeightedGraph {
private:
    int num_vertices; // Сколько всего вершин в графе
    int min_edges;    // Минимум ребер, которые должны выходить из каждой вершины
    vector<vector<int>> adj_matrix; // Матрица, где хранятся веса ребер между вершинами
    mt19937 rng;      // Генератор случайных чисел для случайного графа

    // Делает граф связным: если какие-то вершины не соединены, добавляет ребра
    void ensure_connectivity() {
        while (true) {
            vector<bool> visited(num_vertices, false); // Отмечаем, какие вершины мы уже видели
            queue<int> q; // Очередь для обхода вершин
            q.push(0);    // Начинаем с вершины 0
            visited[0] = true;

            // Обходим граф, чтобы найти все вершины, до которых можно дойти
            while (!q.empty()) {
                int vertex = q.front(); // Берем вершину из очереди
                q.pop();
                for (int i = 0; i < num_vertices; ++i) {
                    if (adj_matrix[vertex][i] > 0 && !visited[i]) { // Если есть ребро и вершина еще не посещена
                        visited[i] = true; // Отмечаем вершину как посещенную
                        q.push(i);         // Добавляем ее в очередь
                    }
                }
            }

            // Проверяем, дошли ли мы до всех вершин
            bool all_reachable = true;
            for (int i = 0; i < num_vertices; ++i) {
                if (!visited[i]) { // Если вершина i осталась недостижимой
                    all_reachable = false;
                    vector<int> reachable; // Собираем вершины, до которых дошли
                    for (int j = 0; j < num_vertices; ++j) {
                        if (visited[j]) reachable.push_back(j);
                    }
                    // Выбираем случайную достижимую вершину и соединяем с недостижимой
                    uniform_int_distribution<int> dist(0, reachable.size() - 1);
                    int connect_to = reachable[dist(rng)];
                    uniform_int_distribution<int> weight_dist(1, 20);
                    int weight = weight_dist(rng); // Вес ребра — случайное число от 1 до 20
                    adj_matrix[connect_to][i] = weight;
                    adj_matrix[i][connect_to] = weight; // Добавляем ребро в обе стороны (граф неориентированный)
                    break; // Повторяем проверку с начала
                }
            }
            if (all_reachable) break; // Если все вершины связаны, выходим
        }
    }

    // Считает, сколько ребер в графе
    [[nodiscard]] int count_edges() const {
        int edges = 0;
        for (int i = 0; i < num_vertices; ++i) {
            for (int j = 0; j < num_vertices; ++j) {
                if (adj_matrix[i][j] > 0) edges++; // Если есть ребро, увеличиваем счетчик
            }
        }
        return edges / 2; // Делим на 2, потому что каждое ребро учтено дважды (i, j и j, i)
    }

    // Проверяет, связный ли граф (можно ли дойти от одной вершины до всех остальных)
    [[nodiscard]] bool is_connected() const {
        vector<bool> visited(num_vertices, false); // Отмечаем посещенные вершины
        queue<int> q; // Очередь для обхода
        q.push(0);    // Начинаем с вершины 0
        visited[0] = true;

        while (!q.empty()) {
            int vertex = q.front(); // Берем вершину из очереди
            q.pop();
            for (int i = 0; i < num_vertices; ++i) {
                if (adj_matrix[vertex][i] > 0 && !visited[i]) { // Если есть ребро и вершина не посещена
                    visited[i] = true; // Отмечаем ее
                    q.push(i);         // Добавляем в очередь
                }
            }
        }

        // Если все вершины посещены, граф связный
        return all_of(visited.begin(), visited.end(), [](bool v) { return v; });
    }

public:
    // Создаем граф с заданным числом вершин и минимальным количеством ребер
    WeightedGraph(int vertices, int min_e)
            : num_vertices(vertices), min_edges(min_e), rng(random_device{}()) {
        adj_matrix = vector<vector<int>>(num_vertices, vector<int>(num_vertices, 0));
        generate_graph();    // Создаем случайный граф
        ensure_connectivity(); // Убеждаемся, что он связный
    }

    // Создает случайный граф
    void generate_graph() {
        for (int i = 0; i < num_vertices; ++i) {
            vector<int> possible_connections; // Список вершин, с которыми можно соединить
            for (int j = 0; j < num_vertices; ++j) {
                if (j != i) possible_connections.push_back(j);
            }
            // Решаем, сколько ребер будет у вершины (от min_edges до 2*min_edges)
            uniform_int_distribution<int> conn_dist(min_edges, min(num_vertices-1, min_edges * 2));
            int num_connections = conn_dist(rng);
            shuffle(possible_connections.begin(), possible_connections.end(), rng); // Перемешиваем вершины

            // Добавляем ребра к случайным вершинам
            for (int j = 0; j < num_connections && j < possible_connections.size(); ++j) {
                int target = possible_connections[j];
                if (adj_matrix[i][target] == 0) { // Если ребра еще нет
                    uniform_int_distribution<int> weight_dist(1, 20);
                    int weight = weight_dist(rng); // Вес ребра — случайное число
                    adj_matrix[i][target] = weight;
                    adj_matrix[target][i] = weight; // Граф неориентированный
                }
            }
        }
    }

    // Показывает матрицу смежности и информацию о графе
    void print_adj_matrix() const {
        cout << "Adjacency matrix for graph with " << num_vertices << " vertices:\n";
        cout << "Debug info: Number of edges = " << count_edges()
             << ", Connected: " << (is_connected() ? "Yes" : "No") << "\n";

        cout << "    ";
        for (int j = 0; j < num_vertices; ++j) cout << setw(3) << j << " ";
        cout << "\n   " << string(num_vertices * 4, '-') << "\n";

        for (int i = 0; i < num_vertices; ++i) {
            cout << setw(2) << i << " |";
            for (int j = 0; j < num_vertices; ++j) {
                cout << setw(3) << adj_matrix[i][j] << " ";
            }
            cout << "\n";
        }
    }

    // Находит минимальное остовное дерево (MST) с помощью алгоритма Прима
    [[nodiscard]] vector<pair<int, int>> prim_mst(int& total_weight) const {
        vector<bool> in_mst(num_vertices, false); // Какие вершины уже в MST
        vector<int> key(num_vertices, INT_MAX);   // Минимальный вес ребра до вершины
        vector<int> parent(num_vertices, -1);     // Родитель вершины в MST
        vector<pair<int, int>> mst_edges;         // Список ребер MST

        key[0] = 0; // Начинаем с вершины 0, вес до нее 0
        total_weight = 0;

        for (int count = 0; count < num_vertices; ++count) {
            // Ищем вершину с минимальным весом, которая еще не в MST
            int u = -1;
            for (int v = 0; v < num_vertices; ++v) {
                if (!in_mst[v] && (u == -1 || key[v] < key[u])) {
                    u = v;
                }
            }

            in_mst[u] = true; // Добавляем вершину в MST
            if (parent[u] != -1) { // Если у вершины есть родитель, добавляем ребро
                mst_edges.emplace_back(parent[u], u);
            }
            total_weight += key[u]; // Прибавляем вес ребра к общему весу

            // Обновляем веса для соседей вершины u
            for (int v = 0; v < num_vertices; ++v) {
                if (adj_matrix[u][v] > 0 && !in_mst[v] && adj_matrix[u][v] < key[v]) {
                    key[v] = adj_matrix[u][v]; // Уменьшаем вес, если нашли путь короче
                    parent[v] = u;             // Запоминаем родителя
                }
            }
        }

        return mst_edges; // Возвращаем ребра MST
    }

    // Показывает минимальное остовное дерево
    void print_mst() const {
        int total_weight = 0;
        vector<pair<int, int>> mst = prim_mst(total_weight);
        cout << "\nMinimum Spanning Tree edges:\n";
        for (const auto& edge : mst) {
            int u = edge.first;
            int v = edge.second;
            cout << "(" << u << ", " << v << ") weight: " << adj_matrix[u][v] << "\n";
        }
        cout << "Total MST weight: " << total_weight << "\n";
    }

    // Замеряет, сколько времени занимает алгоритм Прима
    [[nodiscard]] double measure_prim_time() const {
        int total_weight = 0;
        auto start = chrono::high_resolution_clock::now(); // Засекаем время начала
        static_cast<void>(prim_mst(total_weight)); // Запускаем алгоритм
        auto end = chrono::high_resolution_clock::now();   // Засекаем время конца
        return chrono::duration<double, micro>(end - start).count(); // Считаем разницу в микросекундах
    }
};

// Главная функция программы
int main() {
    int vertex_counts[] = {10, 20, 50, 100}; // Размеры графов для тестов
    int min_edges[] = {3, 4, 10, 20};        // Минимальное число ребер для каждой вершины
    const int num_tests = 10;                // Сколько раз тестируем каждый граф

    cout << "Undirected graphs - Prim's MST Performance Tests:\n";
    vector<vector<double>> test_results(4); // Здесь храним результаты тестов

    for (int i = 0; i < 4; ++i) {
        cout << "\nGraph " << i+1 << " (" << vertex_counts[i] << " vertices):\n";
        WeightedGraph graph(vertex_counts[i], min_edges[i]); // Создаем граф
        graph.print_adj_matrix(); // Показываем матрицу
        graph.print_mst();        // Показываем MST

        cout << "\nRunning " << num_tests << " tests for Prim's MST...\n";
        test_results[i].resize(num_tests);
        for (int t = 0; t < num_tests; ++t) {
            test_results[i][t] = graph.measure_prim_time(); // Замеряем время
            cout << "Test " << t+1 << ": " << fixed << setprecision(3)
                 << test_results[i][t] << " mks\n";
        }

        // Считаем среднее время
        double avg_time = 0;
        for (double time : test_results[i]) avg_time += time;
        avg_time /= num_tests;
        cout << "Average time: " << fixed << setprecision(3) << avg_time << " mks\n";
    }

    // Сохраняем результаты в файл
    ofstream csv_file("prim_performance.csv");
    if (!csv_file.is_open()) {
        cerr << "Error opening CSV file!" << endl;
        return 1;
    }

    csv_file << "N,Average Time (mks)\n";
    for (int i = 0; i < 4; ++i) {
        double avg_time = 0;
        for (double time : test_results[i]) avg_time += time;
        avg_time /= num_tests;
        csv_file << vertex_counts[i] << "," << fixed << setprecision(3) << avg_time << "\n";
    }
    csv_file.close();

    cout << "\nResults saved to prim_performance.csv\n";

    return 0;
}