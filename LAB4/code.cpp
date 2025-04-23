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

// Класс Graph для представления графа
class Graph {
private:
    int vertices; // Количество вершин в графе
    vector<vector<int>> adj_matrix; // Матрица смежности: adj_matrix[i][j] = 1, если есть ребро из i в j
    vector<vector<int>> inc_matrix; // Матрица инцидентности: inc_matrix[i][j] показывает связь вершины i с ребром j
    vector<vector<int>> adj_list; // Список смежности: adj_list[i] содержит вершины, в которые ведут рёбра из i
    vector<pair<int, int>> edge_list; // Список рёбер: хранит пары (from, to) для каждого ребра
    bool is_directed; // Флаг, указывающий, является ли граф направленным

public:
    // Конструктор: инициализирует граф с заданным числом вершин и типом (направленный/ненаправленный)
    Graph(int v, bool dir) : vertices(v), is_directed(dir) {
        adj_matrix.resize(v, vector<int>(v, 0)); // Инициализация матрицы смежности нулями
        inc_matrix.resize(v, vector<int>(0)); // Инициализация матрицы инцидентности (пока пустая)
        adj_list.resize(v); // Инициализация списка смежности
    }

    // Проверка, существует ли ребро от вершины from к вершине to
    [[nodiscard]] bool hasEdge(int from, int to) const {
        return adj_matrix[from][to] == 1; // Возвращает true, если ребро существует
    }

    // Добавление ребра от вершины from к вершине to
    void addEdge(int from, int to) {
        // Отладочный вывод: показываем, какое ребро добавляем
        //cout << "Adding edge (" << from << ", " << to << ")\n";

        /*// Убеждаемся, что не добавляем петлю
        if (from == to) {
            cout << "Warning: Attempted to add a loop (" << from << ", " << to << "). Skipping.\n";
            return;
        }

        // Убеждаемся, что такого ребра ещё нет
        if (hasEdge(from, to)) {
            cout << "Warning: Edge (" << from << ", " << to << ") already exists. Skipping.\n";
            return;
        }*/

        // Добавляем ребро в матрицу смежности
        adj_matrix[from][to] = 1;
        if (!is_directed) adj_matrix[to][from] = 1; // Для ненаправленного графа добавляем обратное ребро

        // Добавляем ребро в список смежности
        adj_list[from].push_back(to);
        if (!is_directed) adj_list[to].push_back(from);

        // Добавляем ребро в список рёбер
        edge_list.emplace_back(from, to);

        // Обновляем матрицу инцидентности: добавляем новый столбец для нового ребра
        for (int i = 0; i < vertices; ++i) {
            if (i == from) inc_matrix[i].push_back(is_directed ? -1 : 1); // Из вершины from ребро выходит (-1 для направленного)
            else if (i == to) inc_matrix[i].push_back(1); // В вершину to ребро входит (1)
            else inc_matrix[i].push_back(0); // Остальные вершины не связаны с этим ребром
        }
    }

    // Вывод матрицы смежности
    void printAdjacencyMatrix() {
        cout << "Adjacency Matrix:\n";
        for (int i = 0; i < vertices; i++) {
            for (int j = 0; j < vertices; j++) {
                cout << adj_matrix[i][j] << " ";
            }
            cout << "\n";
        }
    }

    // Вывод матрицы инцидентности
    void printIncidenceMatrix() {
        cout << "Incidence Matrix:\n";
        int num_edges = inc_matrix[0].size(); // Число рёбер — это число столбцов в матрице
        for (int i = 0; i < vertices; i++) {
            for (int j = 0; j < num_edges; j++) {
                cout << setw(3) << inc_matrix[i][j]; // Выводим значение с выравниванием (ширина 3)
            }
            cout << "\n";
        }
    }

    // Вывод списка смежности
    void printAdjacencyList() {
        cout << "Adjacency List:\n";
        for (int i = 0; i < vertices; i++) {
            cout << i << ": ";
            for (int j : adj_list[i]) {
                cout << j << " ";
            }
            cout << "\n";
        }
    }

    // Вывод списка рёбер
    void printEdgeList() {
        cout << "Edge List:\n";
        for (auto& edge : edge_list) {
            cout << "(" << edge.first << ", " << edge.second << ") ";
        }
        cout << "\n";
    }

    // Получение количества вершин
    [[nodiscard]] int getVertices() const {
        return vertices;
    }

    // Получение списка смежности для заданной вершины
    vector<int>& getAdjList(int v) {
        return adj_list[v];
    }

    // Получение количества рёбер
    [[nodiscard]] int getEdgeCount() const {
        return edge_list.size(); // Размер списка рёбер — это число рёбер в графе
    }
};

// Функция для генерации случайного графа
Graph generateRandomGraph(int v, int e, int max_edges_per_vertex, bool is_directed, int max_in_edges, int max_out_edges) {
    // Инициализация генератора случайных чисел
    random_device rd; // Источник случайности
    mt19937 gen(rd()); // Генератор случайных чисел (Mersenne Twister)

    // Создаём граф с заданным числом вершин и типом (направленный/ненаправленный)
    Graph graph(v, is_directed);

    // Если вершина одна или меньше, добавлять рёбра невозможно, возвращаем пустой граф
    if (v <= 1) return graph;

    // Массивы для отслеживания степеней вершин
    vector<int> out_degrees(v, 0); // Исходящая степень каждой вершины (сколько рёбер выходит из вершины)
    vector<int> in_degrees(v, 0); // Входящая степень каждой вершины (сколько рёбер входит в вершину)
    vector<int> total_degrees(v, 0); // Общая степень каждой вершины (входящие + исходящие рёбра)

    // Создаём равномерное распределение для выбора случайных вершин от 0 до v-1
    uniform_int_distribution<> vertex_choice(0, v - 1);

    // Переменные для контроля процесса добавления рёбер
    int attempts = 0; // Счётчик попыток добавления рёбер
    int max_attempts = 10000; // Максимальное число попыток, чтобы избежать бесконечного цикла
    int added_edges = 0; // Счётчик успешно добавленных рёбер

    // Основной цикл: пытаемся добавить рёбра, пока не достигнем целевого числа e или не превысим max_attempts
    while (added_edges < e && attempts < max_attempts) {
        // Случайно выбираем начальную вершину (from) и конечную вершину (to)
        int from = vertex_choice(gen);
        int to = vertex_choice(gen);
        attempts++; // Увеличиваем счётчик попыток

        // Проверяем, можно ли добавить ребро (from, to). Условия, при которых ребро добавить нельзя:
        // 1. from и to — одна и та же вершина (петли запрещены)
        // 2. Ребро (from, to) уже существует
        // 3. Общее число рёбер, связанных с вершиной from, достигло max_edges_per_vertex
        // 4. Общее число рёбер, связанных с вершиной to, достигло max_edges_per_vertex
        // 5. Для направленного графа: исходящая степень from превышает max_out_edges
        // 6. Для направленного графа: входящая степень to превышает max_in_edges
        if (from == to || graph.hasEdge(from, to) ||
            total_degrees[from] >= max_edges_per_vertex ||
            total_degrees[to] >= max_edges_per_vertex ||
            (is_directed && (out_degrees[from] >= max_out_edges || in_degrees[to] >= max_in_edges))) {
            continue; // Пропускаем попытку, если хотя бы одно из условий выполнено
        }

        // Если все проверки пройдены, добавляем ребро
        graph.addEdge(from, to);

        // Обновляем степени вершин
        out_degrees[from]++; // Увеличиваем исходящую степень вершины from
        total_degrees[from]++; // Увеличиваем общее число рёбер, связанных с from
        if (is_directed) {
            in_degrees[to]++; // Для направленного графа увеличиваем входящую степень to
            total_degrees[to]++; // Увеличиваем общее число рёбер, связанных с to
        } else {
            out_degrees[to]++; // Для ненаправленного графа увеличиваем исходящую степень to
            total_degrees[to]++; // Увеличиваем общее число рёбер, связанных с to
        }
        added_edges++; // Увеличиваем счётчик добавленных рёбер
    }

    // Если не удалось добавить все рёбра за max_attempts попыток, граф всё равно возвращается
    return graph; // Возвращаем сгенерированный граф
}

// Поиск в ширину (BFS) для нахождения кратчайшего пути между start и end
int bfs(Graph& g, int start, int end, vector<int>& path) {
    int v_count = g.getVertices(); // Получаем количество вершин
    vector<int> distance(v_count, -1); // Массив расстояний: -1 означает, что вершина не посещена
    vector<int> parent(v_count, -1); // Массив родителей для восстановления пути
    queue<int> q; // Очередь для BFS

    distance[start] = 0; // Расстояние до начальной вершины равно 0
    q.push(start); // Добавляем начальную вершину в очередь

    // Основной цикл BFS
    while (!q.empty()) {
        int u = q.front(); // Извлекаем вершину из начала очереди
        q.pop();
        for (int v : g.getAdjList(u)) { // Перебираем всех соседей вершины u
            if (distance[v] == -1) { // Если вершина v ещё не посещена
                distance[v] = distance[u] + 1; // Устанавливаем расстояние до v
                parent[v] = u; // Сохраняем родителя v для восстановления пути
                q.push(v); // Добавляем v в очередь
            }
        }
    }

    // Если путь до конечной вершины не найден, возвращаем -1
    if (distance[end] == -1) return -1;

    // Восстанавливаем путь от end до start
    int curr = end;
    while (curr != -1) {
        path.push_back(curr); // Добавляем текущую вершину в путь
        curr = parent[curr]; // Переходим к родителю
    }
    if (!path.empty()) { // Если путь не пустой, разворачиваем его (чтобы он шёл от start к end)
        reverse(path.begin(), path.end());
    }
    return distance[end]; // Возвращаем длину кратчайшего пути
}

// Поиск в глубину (DFS) для нахождения пути между start и end
int dfs(Graph& g, int start, int end, vector<int>& path) {
    int v_count = g.getVertices(); // Получаем количество вершин
    vector<bool> visited(v_count, false); // Массив посещённых вершин
    vector<int> parent(v_count, -1); // Массив родителей для восстановления пути
    stack<int> s; // Стек для итеративного DFS
    s.push(start); // Начинаем с вершины start

    // Основной цикл DFS
    while (!s.empty()) {
        int u = s.top(); // Извлекаем вершину из вершины стека
        s.pop();

        if (u == end) { // Если достигли конечной вершины, завершаем поиск
            visited[u] = true;
            break;
        }

        if (!visited[u]) { // Если вершина u ещё не посещена
            visited[u] = true; // Помечаем её как посещённую
            // Получаем соседей вершины u
            vector<int> neighbors = g.getAdjList(u);
            // Разворачиваем соседей, чтобы порядок обхода был более предсказуемым
            reverse(neighbors.begin(), neighbors.end());
            for (int v : neighbors) { // Перебираем соседей
                if (!visited[v]) { // Если сосед v не посещён
                    parent[v] = u; // Сохраняем родителя v для восстановления пути
                    s.push(v); // Добавляем v в стек
                }
            }
        }
    }

    // Если конечная вершина не посещена, путь не существует
    if (!visited[end]) return -1;

    // Восстанавливаем путь от end до start
    int curr = end;
    while (curr != -1) {
        path.push_back(curr); // Добавляем текущую вершину в путь
        curr = parent[curr]; // Переходим к родителю
    }
    if (!path.empty()) { // Если путь не пустой, разворачиваем его
        reverse(path.begin(), path.end());
    }
    return path.size() - 1; // Возвращаем длину пути
}

// Функция для записи данных о графах в CSV-файл
void generateGraphData(const vector<double>& bfs_times, const vector<double>& dfs_times, const vector<int>& sizes, const vector<bool>& directed, const vector<int>& edges) {
    ofstream file("graph_data.csv"); // Открываем файл для записи
    if (file.is_open()) {
        // Записываем заголовок CSV
        file << "Graph,Vertices,Edges,Directed,BFS_Time,DFS_Time\n";
        // Записываем данные для каждого графа
        for (size_t i = 0; i < sizes.size(); ++i) {
            file << i + 1 << "," << sizes[i] << "," << edges[i] << "," << (directed[i] ? "Yes" : "No") << "," << bfs_times[i] << "," << dfs_times[i] << "\n";
        }
        file.close();
        cout << "Graph data saved to graph_data.csv. Use a plotting tool (e.g., Python) to visualize.\n";
    }
}

int main() {
    random_device rd; // Генератор случайных чисел
    mt19937 gen(rd());

    // Параметры для генерации графов
    int min_vertices = 5, max_vertices = 10; // Диапазон числа вершин
    int min_edges = 10, max_edges = 20; // Диапазон числа рёбер
    int step_vertices = (max_vertices - min_vertices) / 9; // Шаг увеличения числа вершин
    int step_edges = (max_edges - min_edges) / 9; // Шаг увеличения числа рёбер
    int max_edges_per_vertex = 5; // Максимальная общая степень вершины (входящие + исходящие рёбра)
    int max_in_edges = 3; // Максимальное количество входящих рёбер для направленного графа
    int max_out_edges = 3; // Максимальное количество исходящих рёбер для направленного графа

    // Векторы для хранения данных о графах
    vector<int> sizes; // Размеры графов (число вершин)
    vector<bool> directed_flags; // Флаги направленности графов
    vector<Graph> graphs; // Сами графы
    vector<double> bfs_times, dfs_times; // Времена выполнения BFS и DFS
    vector<int> edge_counts; // Количество рёбер в каждом графе

    // Генерация 10 направленных графов
    cout << "Generating Directed Graphs:\n";
    for (int i = 0; i < 10; ++i) {
        // Вычисляем число вершин и рёбер для текущего графа
        int v = min_vertices + i * step_vertices;
        int e = min_edges + i * step_edges;
        e = min(e, v * (v - 1)); // Ограничиваем число рёбер для направленного графа
        sizes.push_back(v); // Сохраняем размер графа
        directed_flags.push_back(true); // Указываем, что граф направленный

        // Генерируем граф
        Graph g = generateRandomGraph(v, e, max_edges_per_vertex, true, max_in_edges, max_out_edges);
        graphs.push_back(g); // Сохраняем граф
        edge_counts.push_back(g.getEdgeCount()); // Сохраняем реальное количество рёбер

        // Выбираем случайные начальную и конечную вершины для поиска пути
        uniform_int_distribution<> vertex_choice(0, v - 1);
        int start = vertex_choice(gen);
        int end = vertex_choice(gen);
        while (start == end) end = vertex_choice(gen); // Гарантируем, что start != end

        // Выполняем BFS
        vector<int> bfs_path;
        auto bfs_start = chrono::high_resolution_clock::now(); // Засекаем время начала
        int bfs_dist = bfs(g, start, end, bfs_path); // Ищем кратчайший путь
        auto bfs_end = chrono::high_resolution_clock::now(); // Засекаем время окончания
        double bfs_time = chrono::duration<double>(bfs_end - bfs_start).count(); // Вычисляем время выполнения
        bfs_times.push_back(bfs_time); // Сохраняем время выполнения BFS
        cout << "Directed Graph " << i + 1 << " (V=" << v << ", E=" << g.getEdgeCount() << "):\n";
        cout << "BFS from " << start << " to " << end << ": ";
        if (bfs_dist == -1) cout << "No path\n"; // Если пути нет
        else {
            cout << "Shortest Distance = " << bfs_dist << ", Path = "; // Выводим длину пути
            for (int vertex : bfs_path) cout << vertex << " "; // Выводим сам путь
        }
        cout << "\nTime: " << bfs_time << "s\n";

        // Выполняем DFS
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

        // Для первого графа выводим дополнительные данные
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
        e = min(e, v * (v - 1) / 2); // Ограничиваем число рёбер для ненаправленного графа
        sizes.push_back(v);
        directed_flags.push_back(false);

        Graph g = generateRandomGraph(v, e, max_edges_per_vertex, false, max_in_edges, max_out_edges);
        graphs.push_back(g);
        edge_counts.push_back(g.getEdgeCount());

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
