#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <random>
#include <chrono>
#include <iomanip>
#include <unordered_set>
#include <cmath>

using namespace std;

// Класс, реализующий алгоритм хеширования RIPEMD-160
class RIPEMD160 {
private:
    uint32_t state[5]{};        // Массив для хранения текущего состояния хэша (5 регистров)
    uint64_t count;           // Счётчик байтов, обработанных алгоритмом
    unsigned char buffer[64]{}; // Временный буфер для данных, пока не наберётся 64 байта

    // Константы для разных раундов преобразований
    static constexpr uint32_t K[5] = {
            0x00000000, 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xA953FD4E
    };

    static constexpr uint32_t KK[5] = {
            0x50A28BE6, 0x5C4DD124, 0x6D703EF3, 0x7A6D76E9, 0x00000000
    };

    // Порядок использования слов сообщения в левой ветви алгоритма
    static constexpr int r[80] = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8,
            3, 10, 14, 4, 9, 15, 8, 1, 2, 7, 0, 6, 13, 11, 5, 12,
            1, 9, 11, 10, 0, 8, 12, 4, 13, 3, 7, 15, 14, 5, 6, 2,
            4, 0, 5, 9, 7, 12, 2, 10, 14, 1, 3, 8, 11, 6, 15, 13
    };

    // Порядок использования слов в правой ветви (параллельная обработка)
    static constexpr int rr[80] = {
            5, 14, 7, 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12,
            6, 11, 3, 7, 0, 13, 5, 10, 14, 15, 8, 12, 4, 9, 1, 2,
            15, 5, 1, 3, 7, 14, 6, 9, 11, 8, 12, 2, 10, 0, 4, 13,
            8, 6, 4, 1, 3, 11, 15, 0, 5, 12, 2, 13, 9, 7, 10, 14,
            12, 15, 10, 4, 1, 5, 8, 7, 6, 2, 13, 14, 0, 3, 9, 11
    };

    // Величины сдвигов для левой ветви в каждом из 80 шагов
    static constexpr int s[80] = {
            11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8,
            7, 6, 8, 13, 11, 9, 7, 15, 7, 12, 15, 9, 11, 7, 13, 12,
            11, 13, 6, 7, 14, 9, 13, 15, 14, 8, 13, 6, 5, 12, 7, 5,
            11, 12, 14, 15, 14, 15, 9, 8, 9, 14, 5, 6, 8, 6, 5, 12,
            9, 15, 5, 11, 6, 8, 13, 12, 5, 12, 13, 14, 11, 8, 5, 6
    };

    // Величины сдвигов для правой ветви
    static constexpr int ss[80] = {
            8, 9, 9, 11, 13, 15, 15, 5, 7, 7, 8, 11, 14, 14, 12, 6,
            9, 13, 15, 7, 12, 8, 9, 11, 7, 7, 12, 7, 6, 15, 13, 11,
            9, 7, 15, 11, 8, 6, 6, 14, 12, 13, 5, 14, 13, 13, 7, 5,
            15, 5, 8, 11, 14, 14, 6, 14, 6, 9, 12, 9, 12, 5, 15, 8,
            8, 5, 12, 9, 12, 5, 14, 6, 8, 13, 6, 5, 15, 13, 11, 11
    };

    // Логические функции для каждого раунда преобразований
    static uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }           // Простое исключающее ИЛИ
    static uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }  // Комбинация И, ИЛИ и НЕ
    static uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return (x | ~y) ^ z; }        // Сложная комбинация с инверсией
    static uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (y & ~z); }  // Другая логическая комбинация
    static uint32_t J(uint32_t x, uint32_t y, uint32_t z) { return x ^ (y | ~z); }        // Финальная функция с ИЛИ и НЕ

    // Циклический сдвиг влево на заданное число бит
    static uint32_t ROL(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }

    // Основная функция преобразования блока данных (64 байта)
    void transform(const unsigned char* data) {
        // Инициализируем временные регистры текущими значениями состояния
        uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
        uint32_t aa = a, bb = b, cc = c, dd = d, ee = e;
        uint32_t x[16]; // Массив для хранения 16 слов по 32 бита из входных данных

        // Преобразуем входной блок в 16 слов (little-endian порядок)
        for(int i = 0; i < 16; i++)
            x[i] = (data[4*i]) | (data[4*i+1] << 8) | (data[4*i+2] << 16) | (data[4*i+3] << 24);

        uint32_t t; // Временная переменная для вычислений
        // 80 шагов преобразования для левой и правой ветвей
        for(int j = 0; j < 80; j++) {
            t = a;
            // Выбор функции и констант зависит от текущего раунда
            if(j < 16)
                t += F(b,c,d) + x[r[j]];
            else if(j < 32)
                t += G(b,c,d) + x[r[j]] + K[1];
            else if(j < 48)
                t += H(b,c,d) + x[r[j]] + K[2];
            else if(j < 64)
                t += I(b,c,d) + x[r[j]] + K[3];
            else
                t += J(b,c,d) + x[r[j]] + K[4];
            // Сдвиг результата и обновление регистров
            t = ROL(t,s[j]) + e;
            a = e; e = d; d = ROL(c,10); c = b; b = t;

            // Аналогичный процесс для правой ветви с другими функциями и перестановками
            t = aa;
            if(j < 16)
                t += J(bb,cc,dd) + x[rr[j]] + KK[4];
            else if(j < 32)
                t += I(bb,cc,dd) + x[rr[j]] + KK[3];
            else if(j < 48)
                t += H(bb,cc,dd) + x[rr[j]] + KK[2];
            else if(j < 64)
                t += G(bb,cc,dd) + x[rr[j]] + KK[1];
            else
                t += F(bb,cc,dd) + x[rr[j]];
            t = ROL(t,ss[j]) + ee;
            aa = ee; ee = dd; dd = ROL(cc,10); cc = bb; bb = t;
        }

        // Обновляем состояние хэша, добавляя результаты обеих ветвей
        t = state[1] + c + dd;
        state[1] = state[2] + d + ee;
        state[2] = state[3] + e + aa;
        state[3] = state[4] + a + bb;
        state[4] = state[0] + b + cc;
        state[0] = t;

        // Очищаем временные данные для безопасности
        memset(x, 0, sizeof(x));
    }

public:
    // Конструктор класса, инициализирует начальные значения состояния
    RIPEMD160() {
        state[0] = 0x67452301;
        state[1] = 0xEFCDAB89;
        state[2] = 0x98BADCFE;
        state[3] = 0x10325476;
        state[4] = 0xC3D2E1F0;
        count = 0;
    }

    // Добавление данных для хеширования
    void update(const unsigned char* data, size_t len) {
        size_t i;
        size_t index = count % 64; // Текущая позиция в буфере
        count += len; // Увеличиваем счётчик обработанных байтов

        // Заполняем буфер данными и обрабатываем, если набирается 64 байта
        for (i = 0; i < len; i++) {
            buffer[index++] = data[i];
            if (index == 64) {
                transform(buffer);
                index = 0;
            }
        }
    }

    // Завершение хеширования и формирование итогового хэша
    string final() {
        unsigned char finalcount[8];
        // Записываем длину сообщения в байтах (little-endian)
        for (int i = 0; i < 8; i++)
            finalcount[i] = static_cast<unsigned char>((count >> (i * 8)) & 255);

        // Добавляем padding: бит 1 и нули до нужной длины
        update((unsigned char*)"\200", 1);
        while ((count % 64) != 56)
            update((unsigned char*)"\0", 1);
        update(finalcount, 8); // Добавляем длину сообщения

        // Формируем итоговый хэш из состояния
        unsigned char digest[20];
        for (int i = 0; i < 20; i++) {
            digest[i] = static_cast<unsigned char>(state[i >> 2] >> ((i & 3) << 3));
        }

        // Преобразуем хэш в строку в шестнадцатеричном формате
        string result;
        result.reserve(40);
        for (unsigned char i : digest) {
            char buf[3];
            sprintf(buf, "%02x", i);
            result += buf;
        }
        return result;
    }

    // Удобный метод для хеширования строки
    string hash(const string& input) {
        update((unsigned char*)input.c_str(), input.length());
        return final();
    }
};

// Поиск длины совпадающего префикса двух строк
size_t find_max_matching_length(const string& str1, const string& str2) {
    size_t max_common_prefix_length = 0;
    // Сравниваем символы двух строк до первого различия
    for (size_t j = 0; j < str1.length(); ++j) {
        if (str1[j] == str2[j]) {
            max_common_prefix_length++;
        } else {
            break;
        }
    }
    return max_common_prefix_length;
}

// Генерация случайной строки заданной длины
string generate_random_string(size_t length) {
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    random_device rd; // Источник случайных чисел
    mt19937 gen(rd()); // Генератор случайных чисел
    uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    string str(length, 0);
    // Заполняем строку случайными символами из заданного набора
    for(size_t i = 0; i < length; ++i) {
        str[i] = charset[dist(gen)];
    }
    return str;
}

// Тест 1: Проверка различий хэшей при изменении строк
void test_string_differences() {
    ofstream out("hash_differences.csv");
    out << "Num Differing Characters,Max Matching Hash Length\n";

    const int variations[] = {1, 2, 4, 8, 16}; // Количество изменяемых символов
    RIPEMD160 hasher;

    // Проверяем для разных количеств изменений
    for(int diff : variations) {
        size_t max_matching = 0;
        for(int i = 0; i < 1000; ++i) {
            string base = generate_random_string(128); // Базовая строка
            string modified = base; // Копия для изменений

            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> dist(0, 127);

            // Вносим заданное количество изменений
            for(int j = 0; j < diff; ++j) {
                int pos = dist(gen);
                modified[pos] = modified[pos] == 'A' ? 'B' : 'A';
            }

            // Вычисляем хэши и сравниваем их
            string hash1 = hasher.hash(base);
            string hash2 = hasher.hash(modified);
            max_matching = max(max_matching, find_max_matching_length(hash1, hash2));
        }

        out << diff << "," << max_matching << "\n";
    }
    out.close();
}

// Тест 2: Поиск коллизий хэшей
void test_hash_collisions() {
    ofstream out("hash_collisions.csv");
    out << "N,Duplicates\n";

    RIPEMD160 hasher;
    // Проверяем для разного количества строк (10^2 до 10^6)
    for(int i = 2; i <= 6; ++i) {
        int N = static_cast<int>(pow(10, i));
        unordered_set<string> unique_hashes; // Множество уникальных хэшей

        // Генерируем строки и хэшируем их
        for(int j = 0; j < N; ++j) {
            string input = generate_random_string(256);
            unique_hashes.insert(hasher.hash(input));
        }

        // Считаем количество коллизий
        int duplicates = N - unique_hashes.size();
        out << N << "," << duplicates << "\n";
    }
    out.close();
}

// Тест 3: Измерение скорости хеширования
void test_hash_speed() {
    ofstream out("hash_speed.csv");
    out << "String Length,Average Time (ms)\n";

    RIPEMD160 hasher;
    // Проверяем для строк разной длины (2^6 до 2^13)
    for(int i = 6; i <= 13; ++i) {
        auto length = static_cast<size_t>(pow(2, i));
        double total_time = 0;

        // Замеряем время для 1000 хэширований
        for(int j = 0; j < 1000; ++j) {
            string input = generate_random_string(length);

            auto start = chrono::high_resolution_clock::now();
            hasher.hash(input);
            auto end = chrono::high_resolution_clock::now();

            total_time += chrono::duration<double, milli>(end - start).count();
        }

        // Вычисляем среднее время
        double avg_time = total_time / 1000.0;
        out << length << "," << fixed << setprecision(6) << avg_time << "\n";
    }
    out.close();
}

// Главная функция программы
int main() {
    cout << "\n=== Running RIPEMD-160 Hash Tests ===\n\n";

    // Запускаем тесты и выводим сообщения о прогрессе
    cout << "1. Testing string differences...\n";
    test_string_differences();
    cout << "   > Test 1 completed successfully\n\n";

    cout << "2. Testing hash collisions...\n";
    test_hash_collisions();
    cout << "   > Test 2 completed successfully\n\n";

    cout << "3. Testing hash performance...\n";
    test_hash_speed();
    cout << "   > Test 3 completed successfully\n\n";

    // Итоговое сообщение и информация о сохранённых результатах
    cout << "\n=== All tests completed successfully ===\n";
    cout << "Results saved to:\n";
    cout << "- hash_differences.csv\n";
    cout << "- hash_collisions.csv\n";
    cout << "- hash_speed.csv\n\n";
    return 0;
}