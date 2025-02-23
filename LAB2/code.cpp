#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <limits>

using namespace std;

struct SortStats {
    int recursion_calls;
    int max_depth;
    int max_extra_memory;
};

void merge(vector<double> &arr, int left, int mid, int right, SortStats &stats) {
    int temp_mem = right - left;
    if (temp_mem > stats.max_extra_memory) {
        stats.max_extra_memory = temp_mem;
    }

    vector<double> result(right - left);
    int it1 = 0, it2 = 0;
    while ((left + it1) < mid && (mid + it2) < right) {
        if (arr[left + it1] < arr[mid + it2]) {
            result[it1 + it2] = arr[left + it1];
            ++it1;
        } else {
            result[it1 + it2] = arr[mid + it2];
            ++it2;
        }
    }
    while ((left + it1) < mid) {
        result[it1 + it2] = arr[left + it1];
        ++it1;
    }
    while ((mid + it2) < right) {
        result[it1 + it2] = arr[mid + it2];
        ++it2;
    }
    for (int i = 0; i < (it1 + it2); ++i) {
        arr[left + i] = result[i];
    }
}

void mergeSort(vector<double> &arr, int left, int right, int depth, SortStats &stats) {
    stats.recursion_calls++;
    if (depth > stats.max_depth) {
        stats.max_depth = depth;
    }

    if ((left + 1) >= right) {
        return;
    }

    int mid = (left + right) / 2;

    mergeSort(arr, left, mid, depth + 1, stats);
    mergeSort(arr, mid, right, depth + 1, stats);

    merge(arr, left, mid, right, stats);
}

int main() {
    int sizes[8] = {1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000};
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> distribution(-1, 1);

    // Векторы для хранения итоговых данных по всем размерам
    vector<double> max_times(8), avg_times(8), min_times(8);
    vector<int> max_rec_calls(8), avg_rec_calls(8), min_rec_calls(8);
    vector<int> max_depths(8), avg_depths(8), min_depths(8);
    vector<int> max_memories(8), avg_memories(8), min_memories(8);

    for (int s = 0; s < 8; ++s) {
        int size = sizes[s];
        vector<double> times(20);
        vector<int> recursion_counts(20);
        vector<int> depths(20);
        vector<int> memories(20);

        for (int k = 0; k < 20; ++k) {
            vector<double> arr(size);
            for (auto &element : arr) {
                element = distribution(gen);
            }

            SortStats stats = {0, 0, 0};
            auto start = chrono::high_resolution_clock::now();
            mergeSort(arr, 0, size, 0, stats);
            auto end = chrono::high_resolution_clock::now();

            // Измеряем время в миллисекундах напрямую
            chrono::duration<double, milli> ms_diff = end - start;
            double time_ms = ms_diff.count();

            times[k] = time_ms;
            recursion_counts[k] = stats.recursion_calls;
            depths[k] = stats.max_depth;
            memories[k] = stats.max_extra_memory;
        }

        // Вычисление средних, минимальных и максимальных значений
        double avg_time = 0, min_time = numeric_limits<double>::max(), max_time = numeric_limits<double>::lowest();
        int avg_rec = 0, min_rec = numeric_limits<int>::max(), max_rec = numeric_limits<int>::lowest();
        int avg_depth = 0, min_depth = numeric_limits<int>::max(), max_depth = numeric_limits<int>::lowest();
        int avg_memory = 0, min_memory = numeric_limits<int>::max(), max_memory = numeric_limits<int>::lowest();

        for (int i = 0; i < 20; ++i) {
            avg_time += times[i];
            min_time = min(min_time, times[i]);
            max_time = max(max_time, times[i]);

            avg_rec += recursion_counts[i];
            min_rec = min(min_rec, recursion_counts[i]);
            max_rec = max(max_rec, recursion_counts[i]);

            avg_depth += depths[i];
            min_depth = min(min_depth, depths[i]);
            max_depth = max(max_depth, depths[i]);

            avg_memory += memories[i];
            min_memory = min(min_memory, memories[i]);
            max_memory = max(max_memory, memories[i]);
        }

        avg_time /= 20;
        avg_rec /= 20;
        avg_depth /= 20;
        avg_memory /= 20;

        // Сохранение данных для текущего размера
        max_times[s] = max_time;
        avg_times[s] = avg_time;
        min_times[s] = min_time;
        max_rec_calls[s] = max_rec;
        avg_rec_calls[s] = avg_rec;
        min_rec_calls[s] = min_rec;
        max_depths[s] = max_depth;
        avg_depths[s] = avg_depth;
        min_depths[s] = min_depth;
        max_memories[s] = max_memory;
        avg_memories[s] = avg_memory;
        min_memories[s] = min_memory;
    }

    // Вывод данных в табличной форме для каждого графика

    // 1. График наихудшего времени
    cout << "\nWorst Time" << endl;
    cout << "Size,Worst Time (ms)" << endl;
    for (int s = 0; s < 8; ++s) {
        cout << sizes[s] << "," << max_times[s] << endl;
    }

    // 2. График среднего, наилучшего и наихудшего времени
    cout << "\nExecution Time (ms)" << endl;
    cout << "Size,Average Time,Best Time,Worst Time" << endl;
    for (int s = 0; s < 8; ++s) {
        cout << sizes[s] << "," << avg_times[s] << "," << min_times[s] << "," << max_times[s] << endl;
    }

    // 3. График среднего, наилучшего и наихудшего числа вызовов
    cout << "\nRecursion Calls" << endl;
    cout << "Size,Avg Calls,Min Calls,Max Calls" << endl;
    for (int s = 0; s < 8; ++s) {
        cout << sizes[s] << "," << avg_rec_calls[s] << "," << min_rec_calls[s] << "," << max_rec_calls[s] << endl;
    }

    // 4. График средней, наилучшей и наихудшей глубины
    cout << "\nRecursion Depth" << endl;
    cout << "Size,Avg Depth,Min Depth,Max Depth" << endl;
    for (int s = 0; s < 8; ++s) {
        cout << sizes[s] << "," << avg_depths[s] << "," << min_depths[s] << "," << max_depths[s] << endl;
    }

    // 5. График среднего, наилучшего и наихудшего потребления памяти
    cout << "\nExtra Memory" << endl;
    cout << "Size,Avg Memory,Min Memory,Max Memory" << endl;
    for (int s = 0; s < 8; ++s) {
        cout << sizes[s] << "," << avg_memories[s] << "," << min_memories[s] << "," << max_memories[s] << endl;
    }

    return 0;
}
