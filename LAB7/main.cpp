#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <vector>
#include <random>
#include <numeric>
#include <map> // Добавляем для подсчёта частот

using namespace std;

// Генератор случайных чисел для использования в тестах
random_device rd;
mt19937 gen(rd());

// Класс AVLTree — реализация сбалансированного AVL-дерева
class AVLTree {
public:
    struct Node {
        int key;
        Node* left;
        Node* right;
        int height;
        Node(int key) : key(key), left(nullptr), right(nullptr), height(1) {}
    };

    AVLTree() : root(nullptr), size(0) {}
    AVLTree(const AVLTree& other) : root(nullptr), size(0) { root = copy(other.root); size = other.size; }
    ~AVLTree() { destroy(root); }

    void insert(int key) {
        if (search(key)) return;
        root = insert(root, key);
        ++size;
    }

    void remove(int key) {
        Node* newRoot = remove(root, key);
        if (newRoot != root || root == nullptr) --size;
        root = newRoot;
    }

    bool search(int key) { return search(root, key); }
    int getMaxDepth() { return getMaxDepth(root); }
    int getSize() { return size; }
    vector<int> getLeafDepths() {
        vector<int> leafDepths;
        collectLeafDepths(root, 1, leafDepths);
        return leafDepths;
    }

    template <typename Func>
    double measureTime(Func func, int operations, const vector<int>& keys) {
        AVLTree tempTree(*this);
        for (int i = 0; i < 1000; ++i) func(tempTree, keys[i % keys.size()]);
        auto start = chrono::steady_clock::now();
        for (int i = 0; i < operations; ++i) func(*this, keys[i % keys.size()]);
        auto end = chrono::steady_clock::now();
        return chrono::duration<double>(end - start).count() / operations;
    }

private:
    Node* root;
    int size;

    Node* copy(Node* node) {
        if (!node) return nullptr;
        Node* newNode = new Node(node->key);
        newNode->height = node->height;
        newNode->left = copy(node->left);
        newNode->right = copy(node->right);
        return newNode;
    }

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    Node* insert(Node* node, int key) {
        if (!node) return new Node(key);
        if (key < node->key) node->left = insert(node->left, key);
        else if (key > node->key) node->right = insert(node->right, key);
        else return node;

        node->height = 1 + max(getHeight(node->left), getHeight(node->right));
        int balance = getBalance(node);

        if (balance > 1 && key < node->left->key) return rightRotate(node);
        if (balance < -1 && key > node->right->key) return leftRotate(node);
        if (balance > 1 && key > node->left->key) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && key < node->right->key) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    Node* remove(Node* root, int key) {
        if (!root) return root;
        if (key < root->key) root->left = remove(root->left, key);
        else if (key > root->key) root->right = remove(root->right, key);
        else {
            if (!root->left) {
                Node* temp = root->right;
                delete root;
                return temp;
            } else if (!root->right) {
                Node* temp = root->left;
                delete root;
                return temp;
            }
            Node* temp = getMinValueNode(root->right);
            root->key = temp->key;
            root->right = remove(root->right, temp->key);
        }

        root->height = 1 + max(getHeight(root->left), getHeight(root->right));
        int balance = getBalance(root);

        if (balance > 1 && getBalance(root->left) >= 0) return rightRotate(root);
        if (balance > 1 && getBalance(root->left) < 0) {
            root->left = leftRotate(root->left);
            return rightRotate(root);
        }
        if (balance < -1 && getBalance(root->right) <= 0) return leftRotate(root);
        if (balance < -1 && getBalance(root->right) > 0) {
            root->right = rightRotate(root->right);
            return leftRotate(root);
        }
        return root;
    }

    bool search(Node* node, int key) {
        if (!node) return false;
        if (key == node->key) return true;
        return key < node->key ? search(node->left, key) : search(node->right, key);
    }

    Node* rightRotate(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = 1 + max(getHeight(y->left), getHeight(y->right));
        x->height = 1 + max(getHeight(x->left), getHeight(x->right));
        return x;
    }

    Node* leftRotate(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = 1 + max(getHeight(x->left), getHeight(x->right));
        y->height = 1 + max(getHeight(y->left), getHeight(y->right));
        return y;
    }

    int getHeight(Node* node) { return node ? node->height : 0; }
    int getBalance(Node* node) { return node ? getHeight(node->left) - getHeight(node->right) : 0; }

    Node* getMinValueNode(Node* node) {
        Node* current = node;
        while (current && current->left) current = current->left;
        return current;
    }

    int getMaxDepth(Node* node) {
        if (!node) return 0;
        return 1 + max(getMaxDepth(node->left), getMaxDepth(node->right));
    }

    void collectLeafDepths(Node* node, int currentDepth, vector<int>& leafDepths) {
        if (!node) return;
        if (!node->left && !node->right) leafDepths.push_back(currentDepth);
        collectLeafDepths(node->left, currentDepth + 1, leafDepths);
        collectLeafDepths(node->right, currentDepth + 1, leafDepths);
    }
};

// Класс RandomizedBST — реализация рандомизированного бинарного дерева поиска
class RandomizedBST {
public:
    struct Node {
        int key;
        Node* left;
        Node* right;
        int size;
        Node(int key) : key(key), left(nullptr), right(nullptr), size(1) {}
    };

    RandomizedBST() : root(nullptr), size(0) {}
    RandomizedBST(const RandomizedBST& other) : root(nullptr), size(0) { root = copy(other.root); size = other.size; }
    ~RandomizedBST() { destroy(root); }

    void insert(int key) {
        if (search(key)) return;
        root = insert(root, key);
        ++size;
    }

    void remove(int key) {
        Node* newRoot = remove(root, key);
        if (newRoot != root || root == nullptr) --size;
        root = newRoot;
    }

    bool search(int key) { return search(root, key); }
    int getMaxDepth() { return getMaxDepth(root); }
    int getSize() { return size; }
    vector<int> getLeafDepths() {
        vector<int> leafDepths;
        collectLeafDepths(root, 1, leafDepths);
        return leafDepths;
    }

    template <typename Func>
    double measureTime(Func func, int operations, const vector<int>& keys) {
        RandomizedBST tempTree(*this);
        for (int i = 0; i < 1000; ++i) func(tempTree, keys[i % keys.size()]);
        auto start = chrono::steady_clock::now();
        for (int i = 0; i < operations; ++i) func(*this, keys[i % keys.size()]);
        auto end = chrono::steady_clock::now();
        return chrono::duration<double>(end - start).count() / operations;
    }

private:
    Node* root;
    int size;

    Node* copy(Node* node) {
        if (!node) return nullptr;
        Node* newNode = new Node(node->key);
        newNode->size = node->size;
        newNode->left = copy(node->left);
        newNode->right = copy(node->right);
        return newNode;
    }

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    int getSize(Node* node) { return node ? node->size : 0; }
    void updateSize(Node* node) {
        if (node) node->size = getSize(node->left) + getSize(node->right) + 1;
    }

    Node* merge(Node* left, Node* right) {
        if (!left) return right;
        if (!right) return left;
        int totalSize = getSize(left) + getSize(right);
        uniform_int_distribution<int> dist(0, totalSize - 1);
        if (dist(gen) < getSize(left)) {
            left->right = merge(left->right, right);
            updateSize(left);
            return left;
        } else {
            right->left = merge(left, right->left);
            updateSize(right);
            return right;
        }
    }

    pair<Node*, Node*> split(Node* node, int key) {
        if (!node) return {nullptr, nullptr};
        if (node->key < key) {
            auto [left, right] = split(node->right, key);
            node->right = left;
            updateSize(node);
            return {node, right};
        } else {
            auto [left, right] = split(node->left, key);
            node->left = right;
            updateSize(node);
            return {left, node};
        }
    }

    Node* insert(Node* node, int key) {
        if (!node) return new Node(key);
        int totalSize = getSize(node) + 1;
        uniform_int_distribution<int> dist(0, totalSize - 1);
        if (dist(gen) == 0) {
            Node* newNode = new Node(key);
            auto [left, right] = split(node, key);
            newNode->left = left;
            newNode->right = right;
            updateSize(newNode);
            return newNode;
        }
        if (key < node->key) node->left = insert(node->left, key);
        else node->right = insert(node->right, key);
        updateSize(node);
        return node;
    }

    Node* remove(Node* node, int key) {
        if (!node) return nullptr;
        if (node->key == key) {
            Node* result = merge(node->left, node->right);
            delete node;
            return result;
        }
        if (key < node->key) node->left = remove(node->left, key);
        else node->right = remove(node->right, key);
        updateSize(node);
        return node;
    }

    bool search(Node* node, int key) {
        if (!node) return false;
        if (key == node->key) return true;
        return key < node->key ? search(node->left, key) : search(node->right, key);
    }

    int getMaxDepth(Node* node) {
        if (!node) return 0;
        return 1 + max(getMaxDepth(node->left), getMaxDepth(node->right));
    }

    void collectLeafDepths(Node* node, int currentDepth, vector<int>& leafDepths) {
        if (!node) return;
        if (!node->left && !node->right) leafDepths.push_back(currentDepth);
        collectLeafDepths(node->left, currentDepth + 1, leafDepths);
        collectLeafDepths(node->right, currentDepth + 1, leafDepths);
    }
};

// Класс RedBlackTree — реализация красно-черного дерева
class RedBlackTree {
public:
    enum Color { RED, BLACK };

    struct Node {
        int key;
        Color color;
        Node* left;
        Node* right;
        Node* parent;
        Node(int key) : key(key), color(RED), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    RedBlackTree() : root(nullptr), size(0) {}
    RedBlackTree(const RedBlackTree& other) : root(nullptr), size(0) { root = copy(other.root, nullptr); size = other.size; }
    ~RedBlackTree() { destroy(root); }

    void insert(int key) {
        Node* node = new Node(key);
        root = insert(root, node);
        fixInsert(node);
        ++size;
    }

    void remove(int key) {
        Node* node = findNode(root, key);
        if (node) {
            remove(node);
            --size;
        }
    }

    bool search(int key) { return findNode(root, key) != nullptr; }
    int getMaxDepth() { return getMaxDepth(root); }
    int getSize() { return size; }
    vector<int> getLeafDepths() {
        vector<int> leafDepths;
        collectLeafDepths(root, 1, leafDepths);
        return leafDepths;
    }

    template <typename Func>
    double measureTime(Func func, int operations, const vector<int>& keys) {
        RedBlackTree tempTree(*this);
        for (int i = 0; i < 1000; ++i) func(tempTree, keys[i % keys.size()]);
        auto start = chrono::steady_clock::now();
        for (int i = 0; i < operations; ++i) func(*this, keys[i % keys.size()]);
        auto end = chrono::steady_clock::now();
        return chrono::duration<double>(end - start).count() / operations;
    }

private:
    Node* root;
    int size;

    Node* copy(Node* node, Node* parent) {
        if (!node) return nullptr;
        Node* newNode = new Node(node->key);
        newNode->color = node->color;
        newNode->parent = parent;
        newNode->left = copy(node->left, newNode);
        newNode->right = copy(node->right, newNode);
        return newNode;
    }

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    Node* insert(Node* root, Node* node) {
        if (!root) return node;
        if (node->key < root->key) {
            root->left = insert(root->left, node);
            if (root->left) root->left->parent = root;
        } else if (node->key > root->key) {
            root->right = insert(root->right, node);
            if (root->right) root->right->parent = root;
        } else {
            delete node;
            return root;
        }
        return root;
    }

    void fixInsert(Node* node) {
        if (!node) return;
        Node* parent = nullptr;
        Node* grandparent = nullptr;
        while (node != root && node->color == RED && node->parent && node->parent->color == RED) {
            parent = node->parent;
            grandparent = parent->parent;
            if (parent == grandparent->left) {
                Node* uncle = grandparent->right;
                if (uncle && uncle->color == RED) {
                    grandparent->color = RED;
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    node = grandparent;
                } else {
                    if (node == parent->right) {
                        rotateLeft(parent);
                        node = parent;
                        parent = node->parent;
                    }
                    rotateRight(grandparent);
                    swap(parent->color, grandparent->color);
                    node = parent;
                }
            } else {
                Node* uncle = grandparent->left;
                if (uncle && uncle->color == RED) {
                    grandparent->color = RED;
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    node = grandparent;
                } else {
                    if (node == parent->left) {
                        rotateRight(parent);
                        node = parent;
                        parent = node->parent;
                    }
                    rotateLeft(grandparent);
                    swap(parent->color, grandparent->color);
                    node = parent;
                }
            }
        }
        if (root) root->color = BLACK;
    }

    Node* findNode(Node* node, int key) {
        if (!node) return nullptr;
        if (key == node->key) return node;
        return key < node->key ? findNode(node->left, key) : findNode(node->right, key);
    }

    void remove(Node* node) {
        if (!node) return;
        Node* child = nullptr;
        Node* parent = node->parent;
        Color color = node->color;
        if (node->left && node->right) {
            Node* replace = getMinValueNode(node->right);
            if (!replace) return;
            node->key = replace->key;
            node = replace;
            parent = node->parent;
            color = node->color;
        }
        if (node->left) child = node->left;
        else child = node->right;
        if (child) child->parent = parent;
        if (!parent) root = child;
        else if (node == parent->left) parent->left = child;
        else parent->right = child;
        if (color == BLACK) fixRemove(child, parent);
        delete node;
    }

    void fixRemove(Node* node, Node* parent) {
        if (!parent) return;
        Node* sibling;
        while (node != root && (!node || node->color == BLACK)) {
            if (node == parent->left) {
                sibling = parent->right;
                if (sibling && sibling->color == RED) {
                    sibling->color = BLACK;
                    parent->color = RED;
                    rotateLeft(parent);
                    sibling = parent->right;
                }
                if ((!sibling || !sibling->left || sibling->left->color == BLACK) &&
                    (!sibling || !sibling->right || sibling->right->color == BLACK)) {
                    if (sibling) sibling->color = RED;
                    node = parent;
                    parent = node->parent;
                } else {
                    if (!sibling || !sibling->right || sibling->right->color == BLACK) {
                        if (sibling && sibling->left) sibling->left->color = BLACK;
                        if (sibling) sibling->color = RED;
                        if (sibling) rotateRight(sibling);
                        sibling = parent->right;
                    }
                    if (sibling) sibling->color = parent->color;
                    parent->color = BLACK;
                    if (sibling && sibling->right) sibling->right->color = BLACK;
                    rotateLeft(parent);
                    node = root;
                }
            } else {
                sibling = parent->left;
                if (sibling && sibling->color == RED) {
                    sibling->color = BLACK;
                    parent->color = RED;
                    rotateRight(parent);
                    sibling = parent->left;
                }
                if ((!sibling || !sibling->left || sibling->left->color == BLACK) &&
                    (!sibling || !sibling->right || sibling->right->color == BLACK)) {
                    if (sibling) sibling->color = RED;
                    node = parent;
                    parent = node->parent;
                } else {
                    if (!sibling || !sibling->left || sibling->left->color == BLACK) {
                        if (sibling && sibling->right) sibling->right->color = BLACK;
                        if (sibling) sibling->color = RED;
                        if (sibling) rotateLeft(sibling);
                        sibling = parent->left;
                    }
                    if (sibling) sibling->color = parent->color;
                    parent->color = BLACK;
                    if (sibling && sibling->left) sibling->left->color = BLACK;
                    rotateRight(parent);
                    node = root;
                }
            }
        }
        if (node) node->color = BLACK;
    }

    void rotateLeft(Node* node) {
        if (!node || !node->right) return;
        Node* right = node->right;
        node->right = right->left;
        if (right->left) right->left->parent = node;
        right->parent = node->parent;
        if (!node->parent) root = right;
        else if (node == node->parent->left) node->parent->left = right;
        else node->parent->right = right;
        right->left = node;
        node->parent = right;
    }

    void rotateRight(Node* node) {
        if (!node || !node->left) return;
        Node* left = node->left;
        node->left = left->right;
        if (left->right) left->right->parent = node;
        left->parent = node->parent;
        if (!node->parent) root = left;
        else if (node == node->parent->right) node->parent->right = left;
        else node->parent->left = left;
        left->right = node;
        node->parent = left;
    }

    Node* getMinValueNode(Node* node) {
        if (!node) return nullptr;
        Node* current = node;
        while (current && current->left) current = current->left;
        return current;
    }

    int getMaxDepth(Node* node) {
        if (!node) return 0;
        return 1 + max(getMaxDepth(node->left), getMaxDepth(node->right));
    }

    void collectLeafDepths(Node* node, int currentDepth, vector<int>& leafDepths) {
        if (!node) return;
        if (!node->left && !node->right) leafDepths.push_back(currentDepth);
        collectLeafDepths(node->left, currentDepth + 1, leafDepths);
        collectLeafDepths(node->right, currentDepth + 1, leafDepths);
    }
};

// Функция для записи гистограммы в файл
void writeHistogramToFile(const string& filename, const map<int, int>& histogram, const string& header) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open " << filename << endl;
        return;
    }
    file << header << "\n";
    for (const auto& [value, count] : histogram) {
        file << value << "," << count << "\n";
    }
    file.close();
}

int main() {
    ofstream outFile("results_random.csv");
    if (!outFile.is_open()) {
        cerr << "Failed to open results_random.csv" << endl;
        return 1;
    }
    outFile << "N,AVL Max Depth,AVL Avg Insert Time,AVL Avg Remove Time,AVL Avg Search Time,"
            << "RBST Max Depth,RBST Avg Insert Time,RBST Avg Remove Time,RBST Avg Search Time,"
            << "RB Max Depth,RB Avg Insert Time,RB Avg Remove Time,RB Avg Search Time\n";

    // Векторы для максимальных высот
    vector<int> avlMaxHeightsLastSeries, rbstMaxHeightsLastSeries, rbMaxHeightsLastSeries;
    // Векторы для всех глубин листьев за все итерации
    vector<int> avlLeafDepthsLastSeries, rbstLeafDepthsLastSeries, rbLeafDepthsLastSeries;

    for (int i = 10; i <= 18; ++i) {
        int N = 1 << i;
        cout << "Running tests for N = " << N << " (Random Data)" << endl;

        double totalInsertAVL = 0, totalRemoveAVL = 0, totalSearchAVL = 0;
        double totalInsertRBST = 0, totalRemoveRBST = 0, totalSearchRBST = 0;
        double totalInsertRB = 0, totalRemoveRB = 0, totalSearchRB = 0;
        int maxDepthAVL = 0, maxDepthRBST = 0, maxDepthRB = 0;

        vector<int> testKeys(N);
        for (int j = 0; j < N; ++j) testKeys[j] = j;
        shuffle(testKeys.begin(), testKeys.end(), gen);

        for (int repeat = 0; repeat < 50; ++repeat) {
            cout << "Repeat " << repeat + 1 << " of 50 for N = " << N << endl;

            vector<int> values(N);
            for (int j = 0; j < N; ++j) values[j] = j;
            shuffle(values.begin(), values.end(), gen);

            AVLTree avl_tree;
            for (int v : values) avl_tree.insert(v);
            totalInsertAVL += avl_tree.measureTime([](AVLTree& t, int val) { t.insert(val); }, 1000, testKeys);
            totalRemoveAVL += avl_tree.measureTime([](AVLTree& t, int val) { t.remove(val); }, 1000, testKeys);
            totalSearchAVL += avl_tree.measureTime([](AVLTree& t, int val) { t.search(val); }, 1000, testKeys);
            maxDepthAVL = max(maxDepthAVL, avl_tree.getMaxDepth());

            RandomizedBST rbst;
            for (int v : values) rbst.insert(v);
            totalInsertRBST += rbst.measureTime([](RandomizedBST& t, int val) { t.insert(val); }, 1000, testKeys);
            totalRemoveRBST += rbst.measureTime([](RandomizedBST& t, int val) { t.remove(val); }, 1000, testKeys);
            totalSearchRBST += rbst.measureTime([](RandomizedBST& t, int val) { t.search(val); }, 1000, testKeys);
            maxDepthRBST = max(maxDepthRBST, rbst.getMaxDepth());

            RedBlackTree rb_tree;
            for (int v : values) rb_tree.insert(v);
            totalInsertRB += rb_tree.measureTime([](RedBlackTree& t, int val) { t.insert(val); }, 1000, testKeys);
            totalRemoveRB += rb_tree.measureTime([](RedBlackTree& t, int val) { t.remove(val); }, 1000, testKeys);
            totalSearchRB += rb_tree.measureTime([](RedBlackTree& t, int val) { t.search(val); }, 1000, testKeys);
            maxDepthRB = max(maxDepthRB, rb_tree.getMaxDepth());

            if (i == 18) {
                cout << "AVL size: " << avl_tree.getSize() << ", leaf count: " << avl_tree.getLeafDepths().size() << endl;
                cout << "RBST size: " << rbst.getSize() << ", leaf count: " << rbst.getLeafDepths().size() << endl;
                cout << "RB size: " << rb_tree.getSize() << ", leaf count: " << rb_tree.getLeafDepths().size() << endl;

                avlMaxHeightsLastSeries.push_back(avl_tree.getMaxDepth());
                rbstMaxHeightsLastSeries.push_back(rbst.getMaxDepth());
                rbMaxHeightsLastSeries.push_back(rb_tree.getMaxDepth());

                vector<int> leafDepthsAVL = avl_tree.getLeafDepths();
                avlLeafDepthsLastSeries.insert(avlLeafDepthsLastSeries.end(), leafDepthsAVL.begin(), leafDepthsAVL.end());

                vector<int> leafDepthsRBST = rbst.getLeafDepths();
                rbstLeafDepthsLastSeries.insert(rbstLeafDepthsLastSeries.end(), leafDepthsRBST.begin(), leafDepthsRBST.end());

                vector<int> leafDepthsRB = rb_tree.getLeafDepths();
                rbLeafDepthsLastSeries.insert(rbLeafDepthsLastSeries.end(), leafDepthsRB.begin(), leafDepthsRB.end());
            }
        }

        double avgInsertAVL = totalInsertAVL / 50, avgRemoveAVL = totalRemoveAVL / 50, avgSearchAVL = totalSearchAVL / 50;
        double avgInsertRBST = totalInsertRBST / 50, avgRemoveRBST = totalRemoveRBST / 50, avgSearchRBST = totalSearchRBST / 50;
        double avgInsertRB = totalInsertRB / 50, avgRemoveRB = totalRemoveRB / 50, avgSearchRB = totalSearchRB / 50;

        outFile << N << "," << maxDepthAVL << "," << avgInsertAVL << "," << avgRemoveAVL << "," << avgSearchAVL << ","
                << maxDepthRBST << "," << avgInsertRBST << "," << avgRemoveRBST << "," << avgSearchRBST << ","
                << maxDepthRB << "," << avgInsertRB << "," << avgRemoveRB << "," << avgSearchRB << "\n";
    }
    outFile.close();

    // Подсчёт частот максимальных высот
    map<int, int> avlMaxHeightHist, rbstMaxHeightHist, rbMaxHeightHist;
    for (int height : avlMaxHeightsLastSeries) avlMaxHeightHist[height]++;
    for (int height : rbstMaxHeightsLastSeries) rbstMaxHeightHist[height]++;
    for (int height : rbMaxHeightsLastSeries) rbMaxHeightHist[height]++;

    // Запись гистограммы максимальных высот
    writeHistogramToFile("AVL_max_heights_random.csv", avlMaxHeightHist, "AVL Height,Frequency");
    writeHistogramToFile("RBST_max_heights_random.csv", rbstMaxHeightHist, "RBST Height,Frequency");
    writeHistogramToFile("RB_max_heights_random.csv", rbMaxHeightHist, "RB Height,Frequency");

    // Подсчёт частот высот веток
    map<int, int> avlLeafDepthHist, rbstLeafDepthHist, rbLeafDepthHist;
    for (int depth : avlLeafDepthsLastSeries) avlLeafDepthHist[depth]++;
    for (int depth : rbstLeafDepthsLastSeries) rbstLeafDepthHist[depth]++;
    for (int depth : rbLeafDepthsLastSeries) rbLeafDepthHist[depth]++;

    // Запись гистограммы высот веток
    writeHistogramToFile("AVL_leaf_depths_random.csv", avlLeafDepthHist, "AVL Depth,Frequency");
    writeHistogramToFile("RBST_leaf_depths_random.csv", rbstLeafDepthHist, "RBST Depth,Frequency");
    writeHistogramToFile("RB_leaf_depths_random.csv", rbLeafDepthHist, "RB Depth,Frequency");

    // Для отсортированных данных
    ofstream outFileSorted("results_sorted.csv");
    if (!outFileSorted.is_open()) {
        cerr << "Failed to open results_sorted.csv" << endl;
        return 1;
    }
    outFileSorted << "N,AVL Max Depth,AVL Avg Insert Time,AVL Avg Remove Time,AVL Avg Search Time,"
                  << "RBST Max Depth,RBST Avg Insert Time,RBST Avg Remove Time,RBST Avg Search Time,"
                  << "RB Max Depth,RB Avg Insert Time,RB Avg Remove Time,RB Avg Search Time\n";

    vector<int> avlMaxHeightsLastSeriesSorted, rbstMaxHeightsLastSeriesSorted, rbMaxHeightsLastSeriesSorted;
    vector<int> avlLeafDepthsLastSeriesSorted, rbstLeafDepthsLastSeriesSorted, rbLeafDepthsLastSeriesSorted;

    for (int i = 10; i <= 18; ++i) {
        int N = 1 << i;
        cout << "Running tests for N = " << N << " (Sorted Data)" << endl;

        double totalInsertAVL = 0, totalRemoveAVL = 0, totalSearchAVL = 0;
        double totalInsertRBST = 0, totalRemoveRBST = 0, totalSearchRBST = 0;
        double totalInsertRB = 0, totalRemoveRB = 0, totalSearchRB = 0;
        int maxDepthAVL = 0, maxDepthRBST = 0, maxDepthRB = 0;

        vector<int> testKeys(N);
        for (int j = 0; j < N; ++j) testKeys[j] = j;
        shuffle(testKeys.begin(), testKeys.end(), gen);

        for (int repeat = 0; repeat < 50; ++repeat) {
            cout << "Repeat " << repeat + 1 << " of 50 for N = " << N << endl;

            vector<int> values(N);
            for (int j = 0; j < N; ++j) values[j] = j;

            AVLTree avl_tree;
            for (int v : values) avl_tree.insert(v);
            totalInsertAVL += avl_tree.measureTime([](AVLTree& t, int val) { t.insert(val); }, 1000, testKeys);
            totalRemoveAVL += avl_tree.measureTime([](AVLTree& t, int val) { t.remove(val); }, 1000, testKeys);
            totalSearchAVL += avl_tree.measureTime([](AVLTree& t, int val) { t.search(val); }, 1000, testKeys);
            maxDepthAVL = max(maxDepthAVL, avl_tree.getMaxDepth());

            RandomizedBST rbst;
            for (int v : values) rbst.insert(v);
            totalInsertRBST += rbst.measureTime([](RandomizedBST& t, int val) { t.insert(val); }, 1000, testKeys);
            totalRemoveRBST += rbst.measureTime([](RandomizedBST& t, int val) { t.remove(val); }, 1000, testKeys);
            totalSearchRBST += rbst.measureTime([](RandomizedBST& t, int val) { t.search(val); }, 1000, testKeys);
            maxDepthRBST = max(maxDepthRBST, rbst.getMaxDepth());

            RedBlackTree rb_tree;
            for (int v : values) rb_tree.insert(v);
            totalInsertRB += rb_tree.measureTime([](RedBlackTree& t, int val) { t.insert(val); }, 1000, testKeys);
            totalRemoveRB += rb_tree.measureTime([](RedBlackTree& t, int val) { t.remove(val); }, 1000, testKeys);
            totalSearchRB += rb_tree.measureTime([](RedBlackTree& t, int val) { t.search(val); }, 1000, testKeys);
            maxDepthRB = max(maxDepthRB, rb_tree.getMaxDepth());

            if (i == 18) {
                cout << "AVL size: " << avl_tree.getSize() << ", leaf count: " << avl_tree.getLeafDepths().size() << endl;
                cout << "RBST size: " << rbst.getSize() << ", leaf count: " << rbst.getLeafDepths().size() << endl;
                cout << "RB size: " << rb_tree.getSize() << ", leaf count: " << rb_tree.getLeafDepths().size() << endl;

                avlMaxHeightsLastSeriesSorted.push_back(avl_tree.getMaxDepth());
                rbstMaxHeightsLastSeriesSorted.push_back(rbst.getMaxDepth());
                rbMaxHeightsLastSeriesSorted.push_back(rb_tree.getMaxDepth());

                vector<int> leafDepthsAVL = avl_tree.getLeafDepths();
                avlLeafDepthsLastSeriesSorted.insert(avlLeafDepthsLastSeriesSorted.end(), leafDepthsAVL.begin(), leafDepthsAVL.end());

                vector<int> leafDepthsRBST = rbst.getLeafDepths();
                rbstLeafDepthsLastSeriesSorted.insert(rbstLeafDepthsLastSeriesSorted.end(), leafDepthsRBST.begin(), leafDepthsRBST.end());

                vector<int> leafDepthsRB = rb_tree.getLeafDepths();
                rbLeafDepthsLastSeriesSorted.insert(rbLeafDepthsLastSeriesSorted.end(), leafDepthsRB.begin(), leafDepthsRB.end());
            }
        }

        double avgInsertAVL = totalInsertAVL / 50, avgRemoveAVL = totalRemoveAVL / 50, avgSearchAVL = totalSearchAVL / 50;
        double avgInsertRBST = totalInsertRBST / 50, avgRemoveRBST = totalRemoveRBST / 50, avgSearchRBST = totalSearchRBST / 50;
        double avgInsertRB = totalInsertRB / 50, avgRemoveRB = totalRemoveRB / 50, avgSearchRB = totalSearchRB / 50;

        outFileSorted << N << "," << maxDepthAVL << "," << avgInsertAVL << "," << avgRemoveAVL << "," << avgSearchAVL << ","
                      << maxDepthRBST << "," << avgInsertRBST << "," << avgRemoveRBST << "," << avgSearchRBST << ","
                      << maxDepthRB << "," << avgInsertRB << "," << avgRemoveRB << "," << avgSearchRB << "\n";
    }
    outFileSorted.close();

    // Подсчёт частот максимальных высот для отсортированных данных
    map<int, int> avlMaxHeightHistSorted, rbstMaxHeightHistSorted, rbMaxHeightHistSorted;
    for (int height : avlMaxHeightsLastSeriesSorted) avlMaxHeightHistSorted[height]++;
    for (int height : rbstMaxHeightsLastSeriesSorted) rbstMaxHeightHistSorted[height]++;
    for (int height : rbMaxHeightsLastSeriesSorted) rbMaxHeightHistSorted[height]++;

    // Запись гистограммы максимальных высот
    writeHistogramToFile("AVL_max_heights_sorted.csv", avlMaxHeightHistSorted, "AVL Height,Frequency");
    writeHistogramToFile("RBST_max_heights_sorted.csv", rbstMaxHeightHistSorted, "RBST Height,Frequency");
    writeHistogramToFile("RB_max_heights_sorted.csv", rbMaxHeightHistSorted, "RB Height,Frequency");

    // Подсчёт частот высот веток для отсортированных данных
    map<int, int> avlLeafDepthHistSorted, rbstLeafDepthHistSorted, rbLeafDepthHistSorted;
    for (int depth : avlLeafDepthsLastSeriesSorted) avlLeafDepthHistSorted[depth]++;
    for (int depth : rbstLeafDepthsLastSeriesSorted) rbstLeafDepthHistSorted[depth]++;
    for (int depth : rbLeafDepthsLastSeriesSorted) rbLeafDepthHistSorted[depth]++;

    // Запись гистограммы высот веток
    writeHistogramToFile("AVL_leaf_depths_sorted.csv", avlLeafDepthHistSorted, "AVL Depth,Frequency");
    writeHistogramToFile("RBST_leaf_depths_sorted.csv", rbstLeafDepthHistSorted, "RBST Depth,Frequency");
    writeHistogramToFile("RB_leaf_depths_sorted.csv", rbLeafDepthHistSorted, "RB Depth,Frequency");

    cout << "Results have been written to files." << endl;
    return 0;
}