#ifndef READ_ARRAY_HEADER
#define READ_ARRAY_HEADER

#include <vector>
#include <cstdint>

namespace util
{

using ArrayType = std::vector<int32_t>;

/*
    Получить массив чисел, запарсив аргументы командной строки. Стратегия функции такова:
    парсим следующие флаги:
        -f <fileName> получить элементы из заданного файла
        -s <size> получить размер из этого аргумента, далее элементы читать из stdin
    Иначе кидаем исключение std::runtime_error. Возвращает вектор входных значений.
*/
ArrayType GetInputArray(char* argv[]);

}

#endif // #define READ_ARRAY_HEADER
