#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <chrono>

// Класс BinaryHeap реализует бинарную кучу
class BinaryHeap {
private:
    std::vector<int> heap; // Вектор для хранения элементов кучи

    // Функция возвращает индекс родителя для узла с индексом i
    int parent(int i) { return (i - 1) / 2; }
    // Функция возвращает индекс левого потомка для узла с индексом i
    int left(int i) { return 2 * i + 1; }
    // Функция возвращает индекс правого потомка для узла с индексом i
    int right(int i) { return 2 * i + 2; }

    // Восстанавливает свойство кучи, двигаясь вниз от узла с индексом i
    void heapifyDown(int i) {
        int size = heap.size();
        while (true) {
            int min = i;
            int l = left(i), r = right(i);
            if (l < size && heap[l] < heap[min]) min = l;
            if (r < size && heap[r] < heap[min]) min = r;
            if (min == i) break;
            std::swap(heap[i], heap[min]);
            i = min;
        }
    }

    // Восстанавливает свойство кучи, двигаясь вверх от узла с индексом i
    void heapifyUp(int i) {
        while (i > 0) {
            int p = parent(i);
            if (heap[p] <= heap[i]) break;
            std::swap(heap[i], heap[p]);
            i = p;
        }
    }

public:
    BinaryHeap() {}

    // Резервирует память под n элементов в векторе кучи
    void reserve(size_t n) {
        heap.reserve(n);
    }

    // Вставляет новое значение в кучу
    void insert(int value) {
        heap.push_back(value);
        heapifyUp(heap.size() - 1);
    }

    // Возвращает минимальный элемент кучи (или -1, если куча пуста)
    int getMin() {
        if (heap.empty()) return -1;
        return heap[0];
    }

    // Удаляет минимальный элемент из кучи
    void deleteMin() {
        if (heap.empty()) return;
        if (heap.size() == 1) {
            heap.pop_back();
            return;
        }
        heap[0] = heap.back();
        heap.pop_back();
        heapifyDown(0);
    }
};

// Класс FibonacciHeap реализует Фибоначчиеву кучу
class FibonacciHeap {
private:
    // Структура узла Фибоначчиевой кучи
    struct Node {
        int value;         // Значение узла
        Node* parent;      // Указатель на родителя
        Node* child;       // Указатель на первого ребёнка
        Node* left;        // Указатель на левого соседа в циклическом списке
        Node* right;       // Указатель на правого соседа в циклическом списке
        int degree;        // Степень узла (число детей)
        bool mark;         // Метка для операции decreaseKey (не используется в этом коде)

        Node(int val) : value(val), parent(nullptr), child(nullptr),
                        left(this), right(this), degree(0), mark(false) {}
    };

    Node* min;                  // Указатель на минимальный узел
    int nodeCount;              // Общее число узлов в куче
    std::vector<Node*> degreeTable; // Вспомогательный массив для consolidate

    // Вставляет узел node в циклический список, начинающийся с list
    void insertToList(Node* list, Node* node) {
        node->left = list;
        node->right = list->right;
        list->right->left = node;
        list->right = node;
    }

    // Связывает узел y с узлом x (y становится ребёнком x)
    void link(Node* y, Node* x) {
        y->left->right = y->right;
        y->right->left = y->left;
        y->parent = x;
        if (!x->child) {
            x->child = y;
            y->left = y;
            y->right = y;
        } else {
            insertToList(x->child, y);
        }
        x->degree++;
        y->mark = false;
    }

    // Объединяет деревья с одинаковыми степенями после удаления минимума
    void consolidate() {
        if (!min) return;
        int maxDegree = static_cast<int>(log2(nodeCount)) + 1;
        degreeTable.assign(maxDegree + 1, nullptr);
        std::vector<Node*> roots;
        Node* current = min;
        do {
            roots.push_back(current);
            current = current->right;
        } while (current != min);
        for (Node* node : roots) {
            Node* x = node;
            int d = x->degree;
            while (d < degreeTable.size() && degreeTable[d]) {
                Node* y = degreeTable[d];
                degreeTable[d] = nullptr;
                if (x->value > y->value) {
                    std::swap(x, y);
                }
                link(y, x);
                d++;
            }
            if (d < degreeTable.size()) {
                degreeTable[d] = x;
            }
        }
        min = nullptr;
        for (Node* node : degreeTable) {
            if (node) {
                if (!min) {
                    min = node;
                    node->left = node;
                    node->right = node;
                } else {
                    insertToList(min, node);
                    if (node->value < min->value) {
                        min = node;
                    }
                }
                node->parent = nullptr;
            }
        }
    }

public:
    FibonacciHeap() : min(nullptr), nodeCount(0) {}

    // Вставляет новое значение в кучу
    void insert(int value) {
        Node* node = new Node(value);
        if (!min) {
            min = node;
        } else {
            insertToList(min, node);
            if (value < min->value) {
                min = node;
            }
        }
        nodeCount++;
    }

    // Возвращает минимальный элемент кучи (или -1, если куча пуста)
    int getMin() {
        if (!min) return -1;
        return min->value;
    }

    // Удаляет минимальный элемент из кучи
    void deleteMin() {
        if (!min) return;
        if (min->child) {
            std::vector<Node*> children;
            Node* child = min->child;
            do {
                children.push_back(child);
                child = child->right;
            } while (child != min->child);
            for (Node* x : children) {
                insertToList(min, x);
                x->parent = nullptr;
            }
        }
        if (min->right == min) {
            delete min;
            min = nullptr;
        } else {
            Node* next = min->right;
            min->left->right = min->right;
            min->right->left = min->left;
            delete min;
            min = next;
            consolidate();
        }
        nodeCount--;
    }

    // Деструктор: очищает память, удаляя все узлы
    ~FibonacciHeap() {
        while (min) deleteMin();
    }
};

// Возвращает текущее время в микросекундах
double getTimeInMicroseconds() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / 1000.0;
}

int main() {
    // Открываем файл для записи результатов
    std::ofstream out("results.csv");

    if (!out.is_open()) {
        std::cerr << "Error opening output file\n";
        return 1;
    }

    // Заголовок CSV файла
    out << "n,operation,binary_time,fibonacci_time\n";

    // Генерация случайных значений для вставки
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    std::vector<int> values(10000000);
    for (int& v : values) {
        v = dis(gen);
    }

    // Векторы для хранения результатов замеров времени
    std::vector<std::pair<int, std::pair<double, double>>> getMinResults;
    std::vector<std::pair<int, std::pair<double, double>>> deleteMinResults;
    std::vector<std::pair<int, std::pair<double, double>>> insertResults;
    std::vector<std::pair<int, std::pair<double, double>>> getMinMaxResults;
    std::vector<std::pair<int, std::pair<double, double>>> deleteMinMaxResults;
    std::vector<std::pair<int, std::pair<double, double>>> insertMaxResults;

    // Цикл по размерам кучи от 10^3 до 10^7
    for (int i = 3; i <= 7; ++i) {
        int n = std::pow(10, i);
        std::cout << "Testing for N = " << n << "\n";

        double binaryGetMinAvg = 0, binaryDeleteMinAvg = 0, binaryInsertAvg = 0;
        double binaryGetMinMax = 0, binaryDeleteMinMax = 0, binaryInsertMax = 0;
        double fibGetMinAvg = 0, fibDeleteMinAvg = 0, fibInsertAvg = 0;
        double fibGetMinMax = 0, fibDeleteMinMax = 0, fibInsertMax = 0;

        // Тестирование бинарной кучи
        std::cout << "Binary Heap...\n";
        try {
            BinaryHeap heap;
            heap.reserve(n);
            int valueIndex = 0;
            std::cout << "  Inserting " << n << " elements...\n";

            for (int j = 0; j < n; ++j) {
                heap.insert(values[valueIndex++ % values.size()]);
            }
            std::cout << "  Binary Heap insertions complete\n";

            // Тест getMin
            double totalTime = 0;
            for (int j = 0; j < 1000; ++j) {
                double start = getTimeInMicroseconds();
                heap.getMin();
                double end = getTimeInMicroseconds();
                totalTime += (end - start);
                binaryGetMinMax = std::max(binaryGetMinMax, end - start);
            }
            binaryGetMinAvg = totalTime / 1000.0;

            // Тест deleteMin
            totalTime = 0;
            BinaryHeap deleteHeap;
            deleteHeap.reserve(n);
            for (int j = 0; j < n; ++j) {
                deleteHeap.insert(values[j % values.size()]);
            }
            for (int j = 0; j < std::min(1000, n); ++j) {
                double start = getTimeInMicroseconds();
                deleteHeap.deleteMin();
                double end = getTimeInMicroseconds();
                totalTime += (end - start);
                binaryDeleteMinMax = std::max(binaryDeleteMinMax, end - start);
            }
            binaryDeleteMinAvg = totalTime / std::min(1000, n);

            // Тест insert
            totalTime = 0;
            BinaryHeap insertHeap;
            insertHeap.reserve(1000);
            for (int j = 0; j < 1000; ++j) {
                double start = getTimeInMicroseconds();
                insertHeap.insert(values[valueIndex++ % values.size()]);
                double end = getTimeInMicroseconds();
                totalTime += (end - start);
                binaryInsertMax = std::max(binaryInsertMax, end - start);
            }
            binaryInsertAvg = totalTime / 1000.0;
        } catch (const std::exception& e) {
            std::cerr << "Error during binary heap operations: " << e.what() << std::endl;
            return 1;
        } catch (...) {
            std::cerr << "Unknown error during binary heap operations" << std::endl;
            return 1;
        }

        // Тестирование Фибоначчиевой кучи
        std::cout << "Fibonacci Heap...\n";
        try {
            FibonacciHeap heap;
            int valueIndex = 0;
            std::cout << "  Inserting " << n << " elements...\n";

            for (int j = 0; j < n; ++j) {
                heap.insert(values[valueIndex++ % values.size()]);
                if (j > 0 && j % (n/10) == 0) {
                    std::cout << "  Inserted " << j << " elements\n";
                }
            }
            std::cout << "  Fibonacci Heap insertions complete\n";

            std::cout << "  Starting GetMin test...\n";
            // Тест getMin
            double totalTime = 0;
            for (int j = 0; j < 1000; ++j) {
                double start = getTimeInMicroseconds();
                heap.getMin();
                double end = getTimeInMicroseconds();
                totalTime += (end - start);
                fibGetMinMax = std::max(fibGetMinMax, end - start);
            }
            fibGetMinAvg = totalTime / 1000.0;

            std::cout << "  Starting DeleteMin test...\n";
            // Тест deleteMin
            totalTime = 0;
            FibonacciHeap deleteHeap;
            std::cout << "  Building heap for DeleteMin test...\n";
            for (int j = 0; j < n; ++j) {
                deleteHeap.insert(values[j % values.size()]);
                if (j > 0 && j % (n/10) == 0) {
                    std::cout << "  Built " << j << " elements for DeleteMin test\n";
                }
            }
            std::cout << "  Starting DeleteMin operations...\n";
            for (int j = 0; j < std::min(1000, n); ++j) {
                if (j > 0 && j % 100 == 0) {
                    std::cout << "  Completed " << j << " DeleteMin operations\n";
                }
                double start = getTimeInMicroseconds();
                deleteHeap.deleteMin();
                double end = getTimeInMicroseconds();
                totalTime += (end - start);
                fibDeleteMinMax = std::max(fibDeleteMinMax, end - start);
            }
            fibDeleteMinAvg = totalTime / std::min(1000, n);

            // Тест insert
            totalTime = 0;
            FibonacciHeap insertHeap;
            for (int j = 0; j < 1000; ++j) {
                double start = getTimeInMicroseconds();
                insertHeap.insert(values[valueIndex++ % values.size()]);
                double end = getTimeInMicroseconds();
                totalTime += (end - start);
                fibInsertMax = std::max(fibInsertMax, end - start);
            }
            fibInsertAvg = totalTime / 1000.0;
        } catch (const std::exception& e) {
            std::cerr << "Error during heap operations: " << e.what() << std::endl;
            return 1;
        } catch (...) {
            std::cerr << "Unknown error occurred during heap operations" << std::endl;
            return 1;
        }

        getMinResults.push_back({n, {binaryGetMinAvg, fibGetMinAvg}});
        deleteMinResults.push_back({n, {binaryDeleteMinAvg, fibDeleteMinAvg}});
        insertResults.push_back({n, {binaryInsertAvg, fibInsertAvg}});
        getMinMaxResults.push_back({n, {binaryGetMinMax, fibGetMinMax}});
        deleteMinMaxResults.push_back({n, {binaryDeleteMinMax, fibDeleteMinMax}});
        insertMaxResults.push_back({n, {binaryInsertMax, fibInsertMax}});
    }

    // Записываем результаты в CSV
    for (const auto& result : getMinResults) {
        out << result.first << ",getMin_avg," << result.second.first << "," << result.second.second << "\n";
    }
    for (const auto& result : deleteMinResults) {
        out << result.first << ",deleteMin_avg," << result.second.first << "," << result.second.second << "\n";
    }
    for (const auto& result : insertResults) {
        out << result.first << ",insert_avg," << result.second.first << "," << result.second.second << "\n";
    }
    for (const auto& result : getMinMaxResults) {
        out << result.first << ",getMin_max," << result.second.first << "," << result.second.second << "\n";
    }
    for (const auto& result : deleteMinMaxResults) {
        out << result.first << ",deleteMin_max," << result.second.first << "," << result.second.second << "\n";
    }
    for (const auto& result : insertMaxResults) {
        out << result.first << ",insert_max," << result.second.first << "," << result.second.second << "\n";
    }

    out.close();
    std::cout << "Results saved to results.csv\n";
    return 0;
}