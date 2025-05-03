#include <iostream>
#include <cmath>
#include <random>
#include <chrono>
#include <fstream>
#include <vector>

using namespace std;

const double PI = 3.14159265358979323846;  // Число Пы

// Целевая функция
double F(double x) {
    return x * x + 10 - 10 * cos(2 * PI * x);
}

// Алгоритм симуляции отжига
double SimulatedAnnealing(double x0, double T0, double Tmin, mt19937& gen) {
    double x = x0;  // Начальная координата
    double f = F(x);  // Значение в начальной координате
    double T = T0;  // Начальная температура
    int k = 1;  // Итерация

    // Распределение Коши для генерации новых координат
    cauchy_distribution<double> cauchy(0.0, 1.0);
    // Вероятность принятия решения
    uniform_real_distribution<double> uniform(0.0, 1.0);

    while (T > Tmin) {
        T = T0 / k;  // Температура на итерации
        // Генерация новой координаты
        double x_new = x + T * cauchy(gen);
        double f_new = F(x_new);  // Значение в новой координате
        // Разность между значением функции в новой и прошлой координатах
        double dF = f_new - f;
        // Условие принятия решения
        if (dF < 0 || uniform(gen) < exp(-dF / T)) {
            x = x_new;
            f = f_new;
        }

        k++;
    }

    return x;
}

int main() {
    random_device rd;
    mt19937 gen(rd());

    double x0 = 1.0;  // Заданная начальная координата
    double T0 = 1000.0;  // Начальная температура

    // Критерий завершения
    double Tmin_start = 1e-1;  // Верхняя граница погрешности
    double Tmin_end = 1e-10;  // Нижняя граница погрешности
    double step = 10.0;  // Шаг для расчета погрешностей

    vector<double> Tmin_values;
    double T = Tmin_start;
    while (T >= Tmin_end) {
        Tmin_values.push_back(T);
        T /= step;
    }
    cout << "Generated T: ";
    for (double T_value : Tmin_values) {
        cout << T_value << " ";
    }
    cout << endl;

    ofstream out("results.csv");
    out << "Tmin,1 / Tmin,Время (мс),x,F(x)\n";

    int i = 1;  // Номер текущей погрешности
    int N = Tmin_values.size();  // Количество погрешностей

    for (double Tmin : Tmin_values) {
        double inverse_Tmin = 1.0 / Tmin;  // Для графиков

        // Вывод прогресса в консоль
        cout << i << "/" << N
             << ": Tmin = " << Tmin << "..." << endl;
        gen.seed(rd());
        auto start = chrono::high_resolution_clock::now();
        double x_result = SimulatedAnnealing(x0, T0, Tmin, gen);
        auto end = chrono::high_resolution_clock::now();
        double time_ms = chrono::duration<double, milli>(end - start).count();

        cout << "Time: " << time_ms << " ms\n" << endl;

        out << Tmin << "," << inverse_Tmin << "," << time_ms << "," << x_result
            << "," << F(x_result) << "\n";

        i++;  // Следующая погрешность
    }
    
    out.close();
    cout << "Results saved to results.csv\n";

    return 0;
}