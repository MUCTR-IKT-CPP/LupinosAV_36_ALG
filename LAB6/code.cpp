#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <fstream>

// Структура узла для BST и AVL-дерева
struct Node {
    int key;           // Значение узла (ключ)
    Node* left;        // Указатель на левое поддерево
    Node* right;       // Указатель на правое поддерево
    int height;        // Высота узла (используется в AVL для балансировки)
    Node(int k) : key(k), left(nullptr), right(nullptr), height(1) {} // Конструктор узла
};

// Класс обычного бинарного дерева поиска (BST)
class BST {
private:
    Node* root; // Корень дерева

    // Рекурсивная вставка ключа в BST
    Node* insert(Node* node, int key) {
        if (!node) return new Node(key); // Если узел пустой, создаём новый
        if (key < node->key) node->left = insert(node->left, key); // Идём влево, если ключ меньше
        else if (key > node->key) node->right = insert(node->right, key); // Идём вправо, если ключ больше
        return node; // Дубликаты игнорируются
    }

    // Рекурсивный поиск ключа в BST
    Node* search(Node* node, int key) {
        if (!node || node->key == key) return node; // Нашли ключ или дошли до конца
        if (key < node->key) return search(node->left, key); // Ищем в левом поддереве
        return search(node->right, key); // Ищем в правом поддереве
    }

    // Поиск минимального узла (самый левый)
    Node* findMin(Node* node) {
        while (node->left) node = node->left; // Идём влево до конца
        return node;
    }

    // Рекурсивное удаление ключа из BST
    Node* remove(Node* node, int key) {
        if (!node) return nullptr; // Если узел пустой, ничего не делаем
        if (key < node->key) node->left = remove(node->left, key); // Идём влево
        else if (key > node->key) node->right = remove(node->right, key); // Идём вправо
        else { // Нашли узел для удаления
            if (!node->left) { // Случай 1: нет левого потомка
                Node* temp = node->right;
                delete node;
                return temp;
            }
            else if (!node->right) { // Случай 2: нет правого потомка
                Node* temp = node->left;
                delete node;
                return temp;
            }
            Node* temp = findMin(node->right); // Случай 3: есть оба потомка
            node->key = temp->key; // Заменяем ключ минимальным из правого поддерева
            node->right = remove(node->right, temp->key); // Удаляем минимальный
        }
        return node;
    }

    // Очистка памяти дерева
    void destroy(Node* node) {
        if (node) {
            destroy(node->left); // Рекурсивно чистим левое поддерево
            destroy(node->right); // Рекурсивно чистим правое поддерево
            delete node; // Удаляем текущий узел
        }
    }

    // Рекурсивный метод для вычисления высоты узла
    int height(Node* node) {
        if (!node) return 0;
        return 1 + std::max(height(node->left), height(node->right));
    }

public:
    BST() : root(nullptr) {} // Конструктор: пустое дерево
    ~BST() { destroy(root); } // Деструктор: очищаем память
    void insert(int key) { root = insert(root, key); } // Публичная вставка
    bool search(int key) { return search(root, key) != nullptr; } // Публичный поиск
    void remove(int key) { root = remove(root, key); } // Публичное удаление
    int getHeight() { return height(root); } // Публичный метод для получения высоты дерева
};

// Класс AVL-дерева (самобалансирующееся BST)
class AVL {
private:
    Node* root; // Корень дерева

    // Получение высоты узла
    int getHeight(Node* node) { return node ? node->height : 0; }

    // Вычисление фактора баланса
    int getBalance(Node* node) { return node ? getHeight(node->left) - getHeight(node->right) : 0; }

    // Правый поворот для балансировки
    Node* rightRotate(Node* y) {
        Node* x = y->left; // Левый потомок становится новым корнем
        Node* T2 = x->right; // Правое поддерево x
        x->right = y; // y становится правым потомком x
        y->left = T2; // T2 становится левым потомком y
        y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1; // Обновляем высоту y
        x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1; // Обновляем высоту x
        return x; // Новый корень
    }

    // Левый поворот для балансировки
    Node* leftRotate(Node* x) {
        Node* y = x->right; // Правый потомок становится новым корнем
        Node* T2 = y->left; // Левое поддерево y
        y->left = x; // x становится левым потомком y
        x->right = T2; // T2 становится правым потомком x
        x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1; // Обновляем высоту x
        y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1; // Обновляем высоту y
        return y; // Новый корень
    }

    // Рекурсивная вставка в AVL
    Node* insert(Node* node, int key) {
        if (!node) return new Node(key); // Создаём новый узел, если пусто
        if (key < node->key) node->left = insert(node->left, key); // Идём влево
        else if (key > node->key) node->right = insert(node->right, key); // Идём вправо
        else return node; // Дубликаты игнорируются

        node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1; // Обновляем высоту
        int balance = getBalance(node); // Проверяем баланс

        // Балансировка: 4 случая
        if (balance > 1 && key < node->left->key) return rightRotate(node); // Левый-левый
        if (balance < -1 && key > node->right->key) return leftRotate(node); // Правый-правый
        if (balance > 1 && key > node->left->key) { // Левый-правый
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && key < node->right->key) { // Правый-левый
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    // Рекурсивный поиск в AVL
    Node* search(Node* node, int key) {
        if (!node || node->key == key) return node;
        if (key < node->key) return search(node->left, key);
        return search(node->right, key);
    }

    // Поиск минимального узла
    Node* findMin(Node* node) {
        while (node->left) node = node->left;
        return node;
    }

    // Рекурсивное удаление в AVL
    Node* remove(Node* node, int key) {
        if (!node) return nullptr;
        if (key < node->key) node->left = remove(node->left, key);
        else if (key > node->key) node->right = remove(node->right, key);
        else {
            if (!node->left) {
                Node* temp = node->right;
                delete node;
                return temp;
            }
            else if (!node->right) {
                Node* temp = node->left;
                delete node;
                return temp;
            }
            Node* temp = findMin(node->right);
            node->key = temp->key;
            node->right = remove(node->right, temp->key);
        }
        if (!node) return node;

        node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1; // Обновляем высоту
        int balance = getBalance(node); // Проверяем баланс

        // Балансировка после удаления
        if (balance > 1 && getBalance(node->left) >= 0) return rightRotate(node);
        if (balance > 1 && getBalance(node->left) < 0) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && getBalance(node->right) <= 0) return leftRotate(node);
        if (balance < -1 && getBalance(node->right) > 0) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    // Очистка памяти дерева
    void destroy(Node* node) {
        if (node) {
            destroy(node->left);
            destroy(node->right);
            delete node;
        }
    }

    // Рекурсивный метод для вычисления высоты узла
    int height(Node* node) {
        if (!node) return 0;
        return 1 + std::max(height(node->left), height(node->right));
    }

public:
    AVL() : root(nullptr) {} // Конструктор
    ~AVL() { destroy(root); } // Деструктор
    void insert(int key) { root = insert(root, key); } // Публичная вставка
    bool search(int key) { return search(root, key) != nullptr; } // Публичный поиск
    void remove(int key) { root = remove(root, key); } // Публичное удаление
    int getHeight() { return height(root); } // Публичный метод для получения высоты дерева
};

// Генерация случайного массива с уникальными значениями
std::vector<int> generateRandomArray(int size) {
    std::vector<int> arr(size);
    for (int i = 0; i < size; ++i) arr[i] = i; // Заполняем числами от 0 до size-1
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(arr.begin(), arr.end(), gen); // Перемешиваем для случайного порядка
    return arr;
}

// Генерация отсортированного массива
std::vector<int> generateSortedArray(int size) {
    std::vector<int> arr(size);
    for (int i = 0; i < size; ++i) arr[i] = i; // Числа от 0 до size-1
    return arr;
}

// Замер времени вставки
std::pair<double, double> measureInsertTime(BST& bst, AVL& avl, const std::vector<int>& arr) {
    auto start = std::chrono::high_resolution_clock::now(); // Начало замера для BST
    for (int num : arr) bst.insert(num);
    auto end = std::chrono::high_resolution_clock::now(); // Конец замера для BST
    std::chrono::duration<double> bstTime = end - start;

    start = std::chrono::high_resolution_clock::now(); // Начало замера для AVL
    for (int num : arr) avl.insert(num);
    end = std::chrono::high_resolution_clock::now(); // Конец замера для AVL
    std::chrono::duration<double> avlTime = end - start;

    return {bstTime.count() / arr.size(), avlTime.count() / arr.size()}; // Время на одну операцию
}

// Замер времени поиска
std::pair<double, double> measureSearchTime(BST& bst, AVL& avl, const std::vector<int>& keys) {
    auto start = std::chrono::high_resolution_clock::now(); // Начало замера для BST
    for (int key : keys) bst.search(key);
    auto end = std::chrono::high_resolution_clock::now(); // Конец замера для BST
    std::chrono::duration<double> bstTime = end - start;

    start = std::chrono::high_resolution_clock::now(); // Начало замера для AVL
    for (int key : keys) avl.search(key);
    end = std::chrono::high_resolution_clock::now(); // Конец замера для AVL
    std::chrono::duration<double> avlTime = end - start;

    return {bstTime.count() / keys.size(), avlTime.count() / keys.size()}; // Время на одну операцию
}

// Замер времени поиска в массиве
double measureArraySearchTime(const std::vector<int>& arr, const std::vector<int>& keys) {
    auto start = std::chrono::high_resolution_clock::now(); // Начало замера
    for (int key : keys) std::find(arr.begin(), arr.end(), key); // Линейный поиск
    auto end = std::chrono::high_resolution_clock::now(); // Конец замера
    std::chrono::duration<double> duration = end - start;
    return duration.count() / keys.size(); // Время на одну операцию
}

// Замер времени удаления
std::pair<double, double> measureDeleteTime(BST& bst, AVL& avl, const std::vector<int>& keys) {
    auto start = std::chrono::high_resolution_clock::now(); // Начало замера для BST
    for (int key : keys) bst.remove(key);
    auto end = std::chrono::high_resolution_clock::now(); // Конец замера для BST
    std::chrono::duration<double> bstTime = end - start;

    start = std::chrono::high_resolution_clock::now(); // Начало замера для AVL
    for (int key : keys) avl.remove(key);
    end = std::chrono::high_resolution_clock::now(); // Конец замера для AVL
    std::chrono::duration<double> avlTime = end - start;

    return {bstTime.count() / keys.size(), avlTime.count() / keys.size()}; // Время на одну операцию
}

int main() {
    const int seriesCount = 10;        // Количество серий (всегда 10)
    const int cyclesPerSeries = 20;    // Циклов в серии (10 случайных + 10 отсортированных для первых 5 серий)
    const int operations = 1000;       // Количество операций поиска и удаления

    // Открываем файлы для записи результатов
    std::ofstream csvFile("results.csv");
    csvFile << "Series,Size,DataType,Cycle,InsertBST,InsertAVL,SearchBST,SearchAVL,SearchArray,DeleteBST,DeleteAVL\n";

    std::ofstream avgFile("averages.csv");
    avgFile << "Size,DataType,InsertBST,InsertAVL,SearchBST,SearchAVL,SearchArray,DeleteBST,DeleteAVL\n";

    // Основной цикл по сериям
    for (int i = 0; i < seriesCount; ++i) {
        int n = 1 << (10 + i); // Размер массива: 2^(10+i), от 1024 до 524288
        std::cout << "Series " << i << ", Size = " << n << std::endl;

        std::random_device rd; // Источник случайности
        std::mt19937 gen(rd()); // Генератор случайных чисел

        // Векторы для хранения времени операций (случайные данные)
        std::vector<double> insertBSTTimesRandom(cyclesPerSeries / 2, 0.0);
        std::vector<double> insertAVLTimesRandom(cyclesPerSeries / 2, 0.0);
        std::vector<double> searchBSTTimesRandom(cyclesPerSeries / 2, 0.0);
        std::vector<double> searchAVLTimesRandom(cyclesPerSeries / 2, 0.0);
        std::vector<double> searchArrayTimesRandom(cyclesPerSeries / 2, 0.0);
        std::vector<double> deleteBSTTimesRandom(cyclesPerSeries / 2, 0.0);
        std::vector<double> deleteAVLTimesRandom(cyclesPerSeries / 2, 0.0);

        // Тестирование случайных данных (10 циклов для всех серий)
        std::cout << "Random Data:" << std::endl;
        for (int j = 0; j < cyclesPerSeries / 2; ++j) {
            std::vector<int> arr = generateRandomArray(n); // Генерируем случайный массив
            BST bst; // Новое BST
            AVL avl; // Новое AVL

            // Замеряем время вставки (нормализуем на одну операцию)
            auto [insertBST, insertAVL] = measureInsertTime(bst, avl, arr);

            // Выводим высоту деревьев для случайных данных
            std::cout << "Random Data - BST Height: " << bst.getHeight() << ", AVL Height: " << avl.getHeight() << std::endl;

            // Генерируем ключи для поиска и удаления из исходного массива
            std::vector<int> searchKeys = arr;
            std::shuffle(searchKeys.begin(), searchKeys.end(), gen);
            searchKeys.resize(operations); // Оставляем только 1000 ключей
            std::vector<int> deleteKeys = arr;
            std::shuffle(deleteKeys.begin(), deleteKeys.end(), gen);
            deleteKeys.resize(operations); // Оставляем только 1000 ключей

            // Замеряем время поиска и удаления
            auto [searchBST, searchAVL] = measureSearchTime(bst, avl, searchKeys);
            double arraySearch = measureArraySearchTime(arr, searchKeys);
            auto [deleteBST, deleteAVL] = measureDeleteTime(bst, avl, deleteKeys);

            // Сохраняем результаты для вычисления средних
            insertBSTTimesRandom[j] = insertBST;
            insertAVLTimesRandom[j] = insertAVL;
            searchBSTTimesRandom[j] = searchBST;
            searchAVLTimesRandom[j] = searchAVL;
            searchArrayTimesRandom[j] = arraySearch;
            deleteBSTTimesRandom[j] = deleteBST;
            deleteAVLTimesRandom[j] = deleteAVL;

            // Записываем в CSV
            csvFile << i << "," << n << ",Random," << j << "," << insertBST << "," << insertAVL << ","
                    << searchBST << "," << searchAVL << "," << arraySearch << "," << deleteBST << "," << deleteAVL << "\n";

            // Выводим в консоль
            std::cout << "Cycle " << j << " - Insert BST: " << insertBST << " s/op, Insert AVL: " << insertAVL << " s/op, "
                      << "Search BST: " << searchBST << " s/op, Search AVL: " << searchAVL << " s/op, "
                      << "Array Search: " << arraySearch << " s/op, "
                      << "Delete BST: " << deleteBST << " s/op, Delete AVL: " << deleteAVL << " s/op" << std::endl;
        }

        // Вычисляем средние значения для случайных данных
        double avgInsertBSTRandom = 0.0, avgInsertAVLRandom = 0.0;
        double avgSearchBSTRandom = 0.0, avgSearchAVLRandom = 0.0, avgSearchArrayRandom = 0.0;
        double avgDeleteBSTRandom = 0.0, avgDeleteAVLRandom = 0.0;
        for (int j = 0; j < cyclesPerSeries / 2; ++j) {
            avgInsertBSTRandom += insertBSTTimesRandom[j];
            avgInsertAVLRandom += insertAVLTimesRandom[j];
            avgSearchBSTRandom += searchBSTTimesRandom[j];
            avgSearchAVLRandom += searchAVLTimesRandom[j];
            avgSearchArrayRandom += searchArrayTimesRandom[j];
            avgDeleteBSTRandom += deleteBSTTimesRandom[j];
            avgDeleteAVLRandom += deleteAVLTimesRandom[j];
        }
        int halfCycles = cyclesPerSeries / 2;
        avgInsertBSTRandom /= halfCycles;
        avgInsertAVLRandom /= halfCycles;
        avgSearchBSTRandom /= halfCycles;
        avgSearchAVLRandom /= halfCycles;
        avgSearchArrayRandom /= halfCycles;
        avgDeleteBSTRandom /= halfCycles;
        avgDeleteAVLRandom /= halfCycles;

        // Записываем средние значения для случайных данных
        avgFile << n << ",Random," << avgInsertBSTRandom << "," << avgInsertAVLRandom << ","
                << avgSearchBSTRandom << "," << avgSearchAVLRandom << "," << avgSearchArrayRandom << ","
                << avgDeleteBSTRandom << "," << avgDeleteAVLRandom << "\n";

        // Тестирование отсортированных данных (только для первых 5 серий)
        if (i < 5) {
            std::vector<double> insertBSTTimesSorted(cyclesPerSeries / 2, 0.0);
            std::vector<double> insertAVLTimesSorted(cyclesPerSeries / 2, 0.0);
            std::vector<double> searchBSTTimesSorted(cyclesPerSeries / 2, 0.0);
            std::vector<double> searchAVLTimesSorted(cyclesPerSeries / 2, 0.0);
            std::vector<double> searchArrayTimesSorted(cyclesPerSeries / 2, 0.0);
            std::vector<double> deleteBSTTimesSorted(cyclesPerSeries / 2, 0.0);
            std::vector<double> deleteAVLTimesSorted(cyclesPerSeries / 2, 0.0);

            std::cout << "Sorted Data:" << std::endl;
            for (int j = 0; j < cyclesPerSeries / 2; ++j) {
                std::vector<int> arr = generateSortedArray(n); // Генерируем отсортированный массив
                BST bst; // Новое BST
                AVL avl; // Новое AVL

                // Замеряем время вставки
                auto [insertBST, insertAVL] = measureInsertTime(bst, avl, arr);

                // Выводим высоту деревьев для отсортированных данных
                std::cout << "Sorted Data - BST Height: " << bst.getHeight() << ", AVL Height: " << avl.getHeight() << std::endl;

                // Генерируем ключи для поиска и удаления из массива
                std::vector<int> searchKeys = arr;
                std::shuffle(searchKeys.begin(), searchKeys.end(), gen);
                searchKeys.resize(operations); // Оставляем только 1000 ключей
                std::vector<int> deleteKeys = arr;
                std::shuffle(deleteKeys.begin(), deleteKeys.end(), gen);
                deleteKeys.resize(operations); // Оставляем только 1000 ключей

                // Замеряем время поиска и удаления
                auto [searchBST, searchAVL] = measureSearchTime(bst, avl, searchKeys);
                double arraySearch = measureArraySearchTime(arr, searchKeys);
                auto [deleteBST, deleteAVL] = measureDeleteTime(bst, avl, deleteKeys);

                // Сохраняем результаты
                insertBSTTimesSorted[j] = insertBST;
                insertAVLTimesSorted[j] = insertAVL;
                searchBSTTimesSorted[j] = searchBST;
                searchAVLTimesSorted[j] = searchAVL;
                searchArrayTimesSorted[j] = arraySearch;
                deleteBSTTimesSorted[j] = deleteBST;
                deleteAVLTimesSorted[j] = deleteAVL;

                // Записываем в CSV
                csvFile << i << "," << n << ",Sorted," << j << "," << insertBST << "," << insertAVL << ","
                        << searchBST << "," << searchAVL << "," << arraySearch << "," << deleteBST << "," << deleteAVL << "\n";

                // Выводим в консоль
                std::cout << "Cycle " << j << " - Insert BST: " << insertBST << " s/op, Insert AVL: " << insertAVL << " s/op, "
                          << "Search BST: " << searchBST << " s/op, Search AVL: " << searchAVL << " s/op, "
                          << "Array Search: " << arraySearch << " s/op, "
                          << "Delete BST: " << deleteBST << " s/op, Delete AVL: " << deleteAVL << " s/op" << std::endl;
            }

            // Вычисляем средние значения для отсортированных данных
            double avgInsertBSTSorted = 0.0, avgInsertAVLSorted = 0.0;
            double avgSearchBSTSorted = 0.0, avgSearchAVLSorted = 0.0, avgSearchArraySorted = 0.0;
            double avgDeleteBSTSorted = 0.0, avgDeleteAVLSorted = 0.0;
            for (int j = 0; j < cyclesPerSeries / 2; ++j) {
                avgInsertBSTSorted += insertBSTTimesSorted[j];
                avgInsertAVLSorted += insertAVLTimesSorted[j];
                avgSearchBSTSorted += searchBSTTimesSorted[j];
                avgSearchAVLSorted += searchAVLTimesSorted[j];
                avgSearchArraySorted += searchArrayTimesSorted[j];
                avgDeleteBSTSorted += deleteBSTTimesSorted[j];
                avgDeleteAVLSorted += deleteAVLTimesSorted[j];
            }
            avgInsertBSTSorted /= halfCycles;
            avgInsertAVLSorted /= halfCycles;
            avgSearchBSTSorted /= halfCycles;
            avgSearchAVLSorted /= halfCycles;
            avgSearchArraySorted /= halfCycles;
            avgDeleteBSTSorted /= halfCycles;
            avgDeleteAVLSorted /= halfCycles;

            // Записываем средние значения для отсортированных данных
            avgFile << n << ",Sorted," << avgInsertBSTSorted << "," << avgInsertAVLSorted << ","
                    << avgSearchBSTSorted << "," << avgSearchAVLSorted << "," << avgSearchArraySorted << ","
                    << avgDeleteBSTSorted << "," << avgDeleteAVLSorted << "\n";
        }
    }

    // Закрываем файлы и выводим сообщение
    csvFile.close();
    avgFile.close();
    std::cout << "Results saved to results.csv\n";
    std::cout << "Average values saved to averages.csv\n";
    return 0;
}
