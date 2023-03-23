#ifndef QUICK_SORT_HEADER
#define QUICK_SORT_HEADER

#include <cstdlib>

namespace util
{

namespace detail
{
    /*
        Делит массив на две части. Возвращает итератор на pivot-элемент
    */
    template <typename Container, typename Compare>
    auto DoPivot(Container& container, typename Container::size_type left, typename Container::size_type right, Compare comp)
    {
        using value_type = typename Container::value_type;
        using size_type = typename Container::size_type;
        
        /*
            Pivot элемент берем средним, чтобы оптимизировать сортировку изначально сортированных (в том числе в обратном порядке)
        */
        value_type pivot = left;
        std::swap(container[left], container[(left + right) / 2]);
        for (size_type i = left + 1; i <= right; ++i)
            if (comp(container[i], container[left]))
                std::swap(container[i], container[++pivot]);
        
        std::swap(container[left], container[pivot]);
        return pivot;
    }
}

/*
    Сортирует контейнер, заданный итератарами begin и end методом quick sort, используя
    компаратор comp, с использованием OpenMP
*/
template <typename Container, typename Compare>
void QuickSort(Container& container, typename Container::size_type left, typename Container::size_type right, Compare comp)
{
    /*
        Алгоритм следующий: делаем выборку элемента, делим массив на две части (все слева меньше элемента,
        все справа больше), далее создаем две таски по сортировке этих частей.
    */

    if (right - left == 1)
        return ; // one elem array should not be sorted

    if (left < right)
    {
        auto pivot = detail::DoPivot(container, left, right, comp);

        #pragma omp task shared(container)
        {
            if (pivot > 0)
                QuickSort(container, left, pivot - 1, comp);
        }

        #pragma omp task shared(container)
            QuickSort(container, pivot + 1, right, comp);

        #pragma omp taskwait
    }
}

}

#endif // #define QUICK_SORT_HEADER
