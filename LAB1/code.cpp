#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <limits>

using namespace std;

struct SortStats {
    long long swap_count;
    int full_passes;
};

SortStats cocktailSort(vector<double> &arr, int size)
{
    int left_border = 0;
    int right_border = size - 1;
    bool flag;

    long long swap_count = 0;
    int full_passes = 0;

    while (left_border <= right_border)
    {
        flag = false;
        ++full_passes;
        for (int i = right_border; i > left_border; --i)
        {
            if (arr[i - 1] > arr[i])
            {
                swap(arr[i - 1], arr[i]);
                flag = true;
                ++swap_count;
            }
        }
        ++left_border;

        for (int i = left_border; i < right_border; ++i)
        {
            if (arr[i] > arr[i + 1])
            {
                swap(arr[i], arr[i + 1]);
                flag = true;
                ++swap_count;
            }
        }
        --right_border;

        if (!flag) break;
    }

    return {swap_count, full_passes};
}

int main() {
    int sizes[8] = {1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000};
    vector<vector<double>> sec_times(8);
    vector<vector<long long>> swap_counts(8);
    vector<vector<int>> full_passes(8);

    vector<double> best_time(8, numeric_limits<double>::max());
    vector<double> worst_time(8, numeric_limits<double>::lowest());
    vector<double> avg_time(8, 0);
    vector<double> avg_swaps(8, 0);
    vector<double> avg_passes(8, 0);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> distribution(-1, 1);

    for (int s = 0; s < 8; ++s)
    {
        for (int k = 0; k < 20; ++k)
        {
            int M = sizes[s];
            vector<double> arr(M);
            for (auto& element: arr)
            {
                element = distribution(gen);
            }

            chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
            SortStats stats = cocktailSort(arr, M);
            chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

            chrono::duration<double> sec_diff = end - start;
            double time_s = sec_diff.count();
            sec_times[s].push_back(sec_diff.count());
            swap_counts[s].push_back(stats.swap_count);
            full_passes[s].push_back(stats.full_passes);

            best_time[s] = min(best_time[s], time_s);
            worst_time[s] = max(worst_time[s], time_s);
            avg_time[s] += time_s;
            avg_swaps[s] += (double)stats.swap_count;
            avg_passes[s] += stats.full_passes;
            cout << "END OF " << k + 1 << " TRY." << endl;
        }
        avg_time[s] /= 20.0;
        avg_swaps[s] /= 20.0;
        avg_passes[s] /= 20.0;
        cout << "==END OF " << sizes[s] << " SIZE OF ARRAY==" << endl;
    }

    cout << "\n=== Sorting Times (Seconds) ===" << endl;
    for (int s = 0; s < 8; ++s)
    {
        cout << "Size " << sizes[s] << ": ";
        for (double time : sec_times[s])
        {
            cout << time << " s, ";
        }
        cout << endl;
    }

    cout << "\n=== Swap Counts ===" << endl;
    for (int s = 0; s < 8; ++s)
    {
        cout << "Size " << sizes[s] << ": ";
        for (auto swaps : swap_counts[s])
        {
            cout << swaps << ", ";
        }
        cout << endl;
    }

    cout << "\n=== Full Passes ===" << endl;
    for (int s = 0; s < 8; ++s)
    {
        cout << "Size " << sizes[s] << ": ";
        for (int passes : full_passes[s])
        {
            cout << passes << ", ";
        }
        cout << endl;
    }

    cout << "\n=== Best Times (Seconds) ===" << endl;
    for (int s = 0; s < 8; ++s)
    {
        cout << sizes[s] << "," << best_time[s] << endl;
    }

    cout << "\n=== Worst Times (Seconds) ===" << endl;
    for (int s = 0; s < 8; ++s)
    {
        cout << sizes[s] << "," << worst_time[s] << endl;
    }

    cout << "\n=== Average Times (Seconds) ===" << endl;
    for (int s = 0; s < 8; ++s)
    {
        cout << sizes[s] << "," << avg_time[s] << endl;
    }

    cout << "\n=== Average Swap Counts ===" << endl;
    for (int s = 0; s < 8; ++s)
    {
        cout << sizes[s] << "," << avg_swaps[s] << endl;
    }

    cout << "\n=== Average Full Passes ===" << endl;
    for (int s = 0; s < 8; ++s)
    {
        cout << sizes[s] << "," << avg_passes[s] << endl;
    }

    return 0;
}
