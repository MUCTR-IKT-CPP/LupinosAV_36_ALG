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

// Класс WeightedGraph представляет неориентированный взвешенный граф
// и реализует алгоритм Прима для построения минимального остовного дерева (MST).
class WeightedGraph {
private:
    int num_vertices; // Количество вершин в графе
    int min_edges;    // Минимальное количество ребер, исходящих из каждой вершины
    vector<vector<int>> adj_matrix; // Матрица смежности для хранения весов ребер
    mt19937 rng;      // Генератор случайных чисел (Mersenne Twister)

    // Метод ensure_connectivity гарантирует, что граф связный.
    // Использует BFS для проверки достижимости всех вершин и добавляет ребра, если граф несвязный.
    void ensure_connectivity() {
        while (true) {
            // Инициализируем массив посещенных вершин
            vector<bool> visited(num_vertices, false);
            queue<int> q; // Очередь для BFS
            q.push(0);    // Начинаем обход с вершины 0
            visited[0] = true;

            // Выполняем BFS для проверки достижимости вершин
            while (!q.empty()) {
                int vertex = q.front(); // Берем вершину из начала очереди
                q.pop();
                // Проверяем все соседние вершины
                for (int i = 0; i < num_vertices; ++i) {
                    // Если есть ребро (вес > 0) и вершина не посещена, добавляем ее в очередь
                    if (adj_matrix[vertex][i] > 0 && !visited[i]) {
                        visited[i] = true;
                        q.push(i);
                    }
                }
            }

            // Проверяем, все ли вершины были посещены
            bool all_reachable = true;
            for (int i = 0; i < num_vertices; ++i) {
                if (!visited[i]) { // Если вершина i недостижима
                    all_reachable = false;
                    // Собираем список достижимых вершин
                    vector<int> reachable;
                    for (int j = 0; j < num_vertices; ++j) {
                        if (visited[j]) reachable.push_back(j);
                    }
                    // Случайно выбираем достижимую вершину для соединения с недостижимой
                    uniform_int_distribution<int> dist(0, reachable.size() - 1);
                    int connect_to = reachable[dist(rng)];
                    // Генерируем случайный вес ребра от 1 до 20
                    uniform_int_distribution<int> weight_dist(1, 20);
                    int weight = weight_dist(rng);
                    // Добавляем ребро между вершинами connect_to и i
                    adj_matrix[connect_to][i] = weight;
                    adj_matrix[i][connect_to] = weight; // Граф неориентированный, добавляем симметрично
                    break; // Прерываем цикл, чтобы заново запустить BFS
                }
            }

            // Если все вершины достижимы, выходим из цикла
            if (all_reachable) break;
        }
    }

    // Метод count_edges подсчитывает количество ребер в графе.
    // Возвращает число ребер, учитывая, что граф неориентированный (каждое ребро учтено дважды в матрице).
    [[nodiscard]] int count_edges() const {
        int edges = 0;
        // Проходим по всей матрице смежности
        for (int i = 0; i < num_vertices; ++i) {
            for (int j = 0; j < num_vertices; ++j) {
                if (adj_matrix[i][j] > 0) { // Если есть ребро
                    edges++;
                }
            }
        }
        return edges / 2; // Делим на 2, так как каждое ребро учтено дважды (i, j и j, i)
    }

    // Метод is_connected проверяет, является ли граф связным.
    // Использует BFS для обхода графа и проверки достижимости всех вершин.
    [[nodiscard]] bool is_connected() const {
        vector<bool> visited(num_vertices, false); // Массив посещенных вершин
        queue<int> q; // Очередь для BFS
        q.push(0);    // Начинаем с вершины 0
        visited[0] = true;

        // Выполняем BFS
        while (!q.empty()) {
            int vertex = q.front();
            q.pop();
            // Проверяем соседей текущей вершины
            for (int i = 0; i < num_vertices; ++i) {
                if (adj_matrix[vertex][i] > 0 && !visited[i]) {
                    visited[i] = true;
                    q.push(i);
                }
            }
        }

        // Проверяем, все ли вершины были посещены
        return all_of(visited.begin(), visited.end(), [](bool v) { return v; });
    }

public:
    // Конструктор класса WeightedGraph.
    // Принимает количество вершин и минимальное количество ребер на вершину.
    WeightedGraph(int vertices, int min_e)
            : num_vertices(vertices), min_edges(min_e), rng(random_device{}()) {
        // Инициализируем матрицу смежности нулями (нет ребер)
        adj_matrix = vector<vector<int>>(num_vertices, vector<int>(num_vertices, 0));
        generate_graph();    // Генерируем случайный граф
        ensure_connectivity(); // Убеждаемся, что граф связный
    }

    // Метод generate_graph генерирует случайный неориентированный взвешенный граф.
    // Каждая вершина соединяется со случайным количеством других вершин (от min_edges до 2*min_edges).
    void generate_graph() {
        for (int i = 0; i < num_vertices; ++i) {
            // Собираем список возможных вершин для соединения (все, кроме i)
            vector<int> possible_connections;
            for (int j = 0; j < num_vertices; ++j) {
                if (j != i) possible_connections.push_back(j);
            }
            // Определяем случайное количество ребер для вершины i
            uniform_int_distribution<int> conn_dist(min_edges, min(num_vertices-1, min_edges * 2));
            int num_connections = conn_dist(rng);
            // Перемешиваем возможные соединения, чтобы выбирать случайные вершины
            shuffle(possible_connections.begin(), possible_connections.end(), rng);

            // Добавляем ребра к выбранным вершинам
            for (int j = 0; j < num_connections && j < possible_connections.size(); ++j) {
                int target = possible_connections[j];
                if (adj_matrix[i][target] == 0) { // Если ребра еще нет
                    // Генерируем случайный вес от 1 до 20
                    uniform_int_distribution<int> weight_dist(1, 20);
                    int weight = weight_dist(rng);
                    adj_matrix[i][target] = weight;
                    adj_matrix[target][i] = weight; // Граф неориентированный
                }
            }
        }
    }

    // Метод print_adj_matrix выводит матрицу смежности графа в консоль.
    // Также выводит отладочную информацию: количество ребер и связность графа.
    void print_adj_matrix() const {
        cout << "Adjacency matrix for graph with " << num_vertices << " vertices:\n";
        cout << "Debug info: Number of edges = " << count_edges()
             << ", Connected: " << (is_connected() ? "Yes" : "No") << "\n";

        // Выводим заголовок с номерами столбцов
        cout << "    ";
        for (int j = 0; j < num_vertices; ++j) {
            cout << setw(3) << j << " ";
        }
        cout << "\n";
        // Выводим разделительную линию
        cout << "   " << string(num_vertices * 4, '-') << "\n";

        // Выводим строки матрицы
        for (int i = 0; i < num_vertices; ++i) {
            cout << setw(2) << i << " |";
            for (int j = 0; j < num_vertices; ++j) {
                cout << setw(3) << adj_matrix[i][j] << " ";
            }
            cout << "\n";
        }
    }

    // Метод prim_mst реализует алгоритм Прима для построения минимального остовного дерева (MST).
    // Возвращает вектор ребер MST в формате пар (u, v).
    // Сложность: O(V^2), где V — количество вершин.
    [[nodiscard]] vector<pair<int, int>> prim_mst() const {
        vector<bool> in_mst(num_vertices, false); // Флаг: включена ли вершина в MST
        vector<int> key(num_vertices, INT_MAX);   // Минимальный вес ребра, соединяющего вершину с MST
        vector<int> parent(num_vertices, -1);     // Родитель вершины в MST
        vector<pair<int, int>> mst_edges;         // Список ребер MST

        key[0] = 0; // Начинаем с вершины 0, ее ключ — 0

        // Проходим по всем вершинам, чтобы включить их в MST
        for (int count = 0; count < num_vertices; ++count) {
            // Ищем вершину с минимальным ключом, которая еще не в MST
            int u = -1;
            for (int v = 0; v < num_vertices; ++v) {
                if (!in_mst[v] && (u == -1 || key[v] < key[u])) {
                    u = v;
                }
            }

            in_mst[u] = true; // Добавляем вершину u в MST

            // Если у вершины есть родитель, добавляем ребро (parent[u], u) в MST
            if (parent[u] != -1) {
                mst_edges.emplace_back(parent[u], u);
            }

            // Обновляем ключи и родителей для соседних вершин
            for (int v = 0; v < num_vertices; ++v) {
                // Если есть ребро (u, v), вершина v не в MST и вес ребра меньше текущего ключа v
                if (adj_matrix[u][v] > 0 && !in_mst[v] && adj_matrix[u][v] < key[v]) {
                    key[v] = adj_matrix[u][v]; // Обновляем ключ
                    parent[v] = u;             // Устанавливаем родителя
                }
            }
        }

        return mst_edges; // Возвращаем ребра MST
    }

    // Метод print_mst выводит ребра минимального остовного дерева и его общий вес.
    void print_mst() const {
        vector<pair<int, int>> mst = prim_mst(); // Получаем ребра MST
        cout << "\nMinimum Spanning Tree edges:\n";
        int total_weight = 0; // Общий вес MST
        // Проходим по всем ребрам MST
        for (const auto& edge : mst) {
            int u = edge.first;
            int v = edge.second;
            cout << "(" << u << ", " << v << ") weight: " << adj_matrix[u][v] << "\n";
            total_weight += adj_matrix[u][v]; // Суммируем веса ребер
        }
        cout << "Total MST weight: " << total_weight << "\n";
    }

    // Метод measure_prim_time измеряет время выполнения алгоритма Прима.
    // Возвращает время в микросекундах.
    [[nodiscard]] double measure_prim_time() const {
        auto start = chrono::high_resolution_clock::now(); // Засекаем начальное время
        static_cast<void>(prim_mst()); // Вызываем prim_mst, игнорируем результат (нужен только для замера времени)
        auto end = chrono::high_resolution_clock::now(); // Засекаем конечное время
        // Вычисляем разницу времени в микросекундах
        return chrono::duration<double, micro>(end - start).count();
    }
};

// Главная функция программы
int main() {
    // Массивы с параметрами для тестов: количество вершин и минимальное количество ребер
    int vertex_counts[] = {10, 20, 50, 100};
    int min_edges[] = {3, 4, 10, 20};
    const int num_tests = 10; // Количество тестов для каждого графа

    cout << "Undirected graphs - Prim's MST Performance Tests:\n";
    vector<vector<double>> test_results(4); // Массив для хранения результатов тестов

    // Проходим по всем размерам графа
    for (int i = 0; i < 4; ++i) {
        cout << "\nGraph " << i+1 << " (" << vertex_counts[i] << " vertices):\n";
        // Создаем граф с заданным количеством вершин и минимальным количеством ребер
        WeightedGraph graph(vertex_counts[i], min_edges[i]);
        graph.print_adj_matrix(); // Выводим матрицу смежности
        graph.print_mst();        // Выводим MST

        // Проводим тесты производительности
        cout << "\nRunning " << num_tests << " tests for Prim's MST...\n";
        test_results[i].resize(num_tests); // Инициализируем массив для результатов
        for (int t = 0; t < num_tests; ++t) {
            test_results[i][t] = graph.measure_prim_time(); // Замеряем время
            // Выводим результат теста с точностью до 3 знаков после запятой
            cout << "Test " << t+1 << ": " << fixed << setprecision(3)
                 << test_results[i][t] << " mks\n";
        }

        // Вычисляем среднее время выполнения
        double avg_time = 0;
        for (double time : test_results[i]) {
            avg_time += time;
        }
        avg_time /= num_tests;
        cout << "Average time: " << fixed << setprecision(3) << avg_time << " mks\n";
    }

    // Сохраняем результаты в CSV-файл
    ofstream csv_file("prim_performance.csv");
    if (!csv_file.is_open()) {
        cerr << "Error opening CSV file!" << endl;
        return 1;
    }

    // Записываем заголовок CSV
    csv_file << "N,Average Time (mks)\n";
    for (int i = 0; i < 4; ++i) {
        double avg_time = 0;
        for (double time : test_results[i]) {
            avg_time += time;
        }
        avg_time /= num_tests;
        // Записываем среднее время для каждого размера графа
        csv_file << vertex_counts[i] << "," << fixed << setprecision(3) << avg_time << "\n";
    }
    csv_file.close();

    cout << "\nResults saved to prim_performance.csv\n";

    return 0;
}