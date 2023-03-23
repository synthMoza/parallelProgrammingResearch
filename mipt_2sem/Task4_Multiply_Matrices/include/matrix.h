#pragma once

#include <iostream>
#include <cmath>
#include <cassert>

namespace util {
    // This matrix is fully usable for int/float, but not for any other classes
    template <typename T>
    class Matrix final {
        const double eps = 1E-06;
        // The size of the matrix
        int nrows_;
        int ncolumns_;
        // The array of pointers
        T **data_;

        // For usability of A[][]
        struct ProxyRow {
            T *row;
            const T &operator[](int n) const { return row[n]; }
            T &operator[](int n) { return row[n]; }
        };
    public:
        //! Constructor of the 'nrows * ncolumns' matrix filled with the value
        Matrix(int nrows, int ncolumns, T value = T{});

        int GetRows() const { return nrows_; }
        int GetColumns() const { return ncolumns_; }

        //! Print the matrix
        void Print() const;

        //! Get the eye matrix
        static Matrix Eye(int n);
        //! Get zero matrix m * n
        static Matrix Zero(int n, int m);
        //! Get the rotation matrix of size n and between axis of angle
        static Matrix<float> Rotation(int n, int i, int j, float angle);
        //! Negate the matrix
        Matrix &Negate() &;
        //! Transpose the matrix
        Matrix &Transpose() &;
        //! Inverse the matrix
        Matrix &Inverse() &;
        //! Get the negative matrix
        Matrix operator-() const;
        //! Get the matrix of cofactors
        Matrix Cofactors() const;
        //! Remove a row/column from the matrix
        Matrix & RemoveRow(int n) &;
        Matrix & RemoveColumn(int n) &;
        //! Swap matrix rows
        Matrix & SwapRows(int i, int j) &;

        //! Get the minor M(i, j)
        T GetMinor(int i, int j) const;

        // Chained operators
        Matrix &operator+=(const Matrix &rhs) &;
        Matrix &operator-=(const Matrix &rhs) &;
        Matrix &operator*=(int n) &;
        Matrix &operator^=(int n) &;
        Matrix &operator*=(const Matrix &rhs) &;

        //! Usability: A[][]
        ProxyRow operator[](int n);
        ProxyRow operator[](int n) const;

        //! Get the det(A)
        T Determinant() const;
        //! Row_i = Row_i - j_k * Row_j;
        Matrix& SubRows(int i, int j, T j_k) &;
        //! Find the maximum value in the given column (return row of this element)
        int ColumnMax(int j) const;

        //! Template copy constructor
        template <typename U>
        Matrix(const Matrix<U> &rhs) : 
            data_(new T *[nrows_]),
            nrows_(rhs.GetRows()), 
            ncolumns_(rhs.GetColumns()) {
            for (int i = 0; i < nrows_; ++i) {
                data_[i] = new T[ncolumns_];
                for (int j = 0; j < ncolumns_; ++j) {
                    data_[i][j] = rhs[i][j];
                }
            }
        }

        //! Copy constructor
        Matrix(const Matrix& rhs) : 
            data_(new T *[nrows_]),
            nrows_(rhs.GetRows()), 
            ncolumns_(rhs.GetColumns()) {
            for (int i = 0; i < nrows_; ++i) {
                data_[i] = new T[ncolumns_];
                for (int j = 0; j < ncolumns_; ++j) {
                    data_[i][j] = rhs[i][j];
                }
            }
        }

        Matrix &operator=(const Matrix &rhs);        

        // Destroy the matrix 
        void Destroy();
        ~Matrix();
    };

    template <typename T>
    Matrix<T>::Matrix(int nrows, int ncolumns, T value) : 
        data_(new T *[nrows]),
        nrows_(nrows),
        ncolumns_(ncolumns) {
        for (int i = 0; i < nrows_; ++i)
        {
            data_[i] = new T[ncolumns];
            for (int j = 0; j < ncolumns_; ++j)
            {
                data_[i][j] = value;
            }
        }
    }

    template <typename T>
    Matrix<T> &Matrix<T>::operator=(const Matrix &rhs)
    {
        // If we have 'a = a', may return quickly
        if (&rhs == this)
            return *this;

        // If the size of the other matrix is the same, don't reinitilize
        if (nrows_ != rhs.nrows_ || ncolumns_ != rhs.ncolumns_)
        {
            Destroy();

            nrows_ = rhs.nrows_;
            ncolumns_ = rhs.ncolumns_;
            data_ = new T *[nrows_];
        }

        // Copy all values from the right matrix
        for (int i = 0; i < nrows_; ++i)
        {
            data_[i] = new T[ncolumns_];
            for (int j = 0; j < ncolumns_; ++j)
                data_[i][j] = rhs.data_[i][j];
        }

        return *this;
    }

    template <typename T>
    void Matrix<T>::Destroy() {
        for (int i = 0; i < nrows_; ++i)
            delete[] data_[i];

        delete[] data_;
    }

    template <typename T>
    Matrix<T>::~Matrix() {
        Destroy();
    }

    template <typename T>
    void Matrix<T>::Print() const
    {
        std::cout << "Rows: " << nrows_ << std::endl;
        std::cout << "Columns: " << ncolumns_ << std::endl;

        for (int i = 0; i < nrows_; ++i)
        {
            for (int j = 0; j < ncolumns_; ++j)
            {
                std::cout << data_[i][j] << " ";
            }

            std::cout << std::endl;
        }
    }

    template <typename T>
    Matrix<T> Matrix<T>::Eye(int n)
    {
        Matrix res{n, n, 0};

        for (int i = 0; i < n; ++i)
        {
            res.data_[i][i] = 1;
        }

        return res;
    }
    template <typename T>
    Matrix<T> &Matrix<T>::Negate() &
    {
        for (int i = 0; i < nrows_; ++i)
            for (int j = 0; j < ncolumns_; ++j)
                data_[i][j] *= -1;

        return *this;
    }

    template <typename T>
    Matrix<T> Matrix<T>::operator-() const {
        Matrix<T> tmp = *this;
        return tmp.negate();
    }

    template <typename T>
    typename Matrix<T>::ProxyRow Matrix<T>::operator[](int n) {
        ProxyRow temp_row;
        temp_row.row = data_[n];
        return temp_row;
    }

    template <typename T>
    typename Matrix<T>::ProxyRow Matrix<T>::operator[](int n) const {
        ProxyRow temp_row;
        temp_row.row = data_[n];
        return temp_row;
    }

    template <typename T>
    T Matrix<T>::Determinant() const {
        assert(ncolumns_ == nrows_);

        // Gauss elimination
        Matrix<long double> temp{*this};

        int sign = 1;
        long double res = 1;
        int max = 0;
        long double max_val = 0;

        for (int i = 0; i < nrows_ - 1; ++i) {
            // Find the maximum value in the current column
            // and put the max to the top (if neccesary)
            max = temp.ColumnMax(i);
            max_val = temp[max][i];
            if (max != i) {
                temp.SwapRows(max, i);
                sign *= -1;
            }
            
            if (max_val == 0)
                return 0;

            // Zero all elements under this maximum value
            for (int j = i + 1; j < nrows_; ++j) {
                temp.SubRows(j, i, temp[j][i] / max_val);
            }

            res *= temp[i][i];
        }

        res *= (sign * temp[nrows_ - 1][nrows_ - 1]);
        return static_cast<T>(res);
    }

    template <typename T>
    Matrix<T>& Matrix<T>::operator+=(const Matrix &rhs) & {
        assert(ncolumns_ == rhs.ncolumns_ && nrows_ == rhs.nrows_);

        for (int i = 0; i < nrows_; ++i) {
            for (int j = 0; j < ncolumns_; ++j) {
                data_[i][j] += rhs.data_[i][j];
            }
        }

        return *this;
    }

    template <typename T>
    Matrix<T> &Matrix<T>::operator-=(const Matrix &rhs) &
    {
        assert(ncolumns_ == rhs.ncolumns_ && nrows_ == rhs.nrows_);

        for (int i = 0; i < nrows_; ++i)
        {
            for (int j = 0; j < ncolumns_; ++j)
            {
                data_[i][j] -= rhs.data_[i][j];
            }
        }

        return *this;
    }

    template <typename T>
    Matrix<T> &Matrix<T>::operator*=(int n) &
    {
        for (int i = 0; i < nrows_; ++i)
        {
            for (int j = 0; j < ncolumns_; ++j)
            {
                data_[i][j] *= n;
            }
        }

        return *this;
    }

    template <typename T>
    Matrix<T>& Matrix<T>::operator*=(const Matrix &rhs) & {
        assert(ncolumns_ == rhs.nrows_);

        Matrix<T> res{nrows_, rhs.ncolumns_, 0};

        #pragma omp parallel for schedule(guided) shared(data_, rhs, res)
        for (int i = 0; i < nrows_; ++i) {
            T* r_res = &res.data_[i][0];
            for (int k = 0; k < ncolumns_; ++k) {
                const T* r_rhs = &rhs.data_[k][0];
                T r_lhs = data_[i][k];

                for (int j = 0; j < rhs.ncolumns_; ++j) {
                    r_res[j] += r_lhs * r_rhs[j];
                }
            }
        }

        *this = res;
        return *this;
    }

    //! Returns true if the matrix's values and size are equal
    template <typename T>
    bool operator==(const Matrix<T> &lhs, const Matrix<T> &rhs) {
        if (lhs.GetRows() != rhs.GetRows() || lhs.GetColumns() != rhs.GetColumns())
            return false;

        if (&rhs == &lhs)
            return true;

        int rows = rhs.GetRows();
        int columns = rhs.GetColumns();

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                if (lhs[i][j] != rhs[i][j])
                    return false;
            }
        }

        return true;
    }

    template <typename T>
    bool operator!=(const Matrix<T> &lhs, const Matrix<T> &rhs) {
        return !(operator==(lhs, rhs));
    }

        template <typename T>
        Matrix<T> operator+(const Matrix<T> &lhs, const Matrix<T> &rhs)
    {
        Matrix<T> res{lhs};
        res += rhs;
        return res;
    }

    template <typename T>
    Matrix<T> operator-(const Matrix<T> &lhs, const Matrix<T> &rhs) {
        Matrix<T> res{lhs};
        res -= rhs;
        return res;
    }   

    template <typename T>
    Matrix<T> operator*(const Matrix<T> &lhs, int n) {
        Matrix<T> res{lhs};
        res *= n;
        return res;
    }   

    template <typename T>
    Matrix<T>& Matrix<T>::operator^=(int n) & {
        Matrix<T> mul{*this};
        for (int i = 1; i < n; ++i) {
            (*this) *= mul;
        }

        return *this;
    }  

    template <typename T>
    Matrix<T> operator^(const Matrix<T> &lhs, int n) {
        Matrix<T> res = {lhs};
        res ^= n;
        return res;
    }   

    template <typename T>
    Matrix<T> operator*(const Matrix<T> &lhs, const Matrix<T> &rhs) {
        Matrix<T> res{lhs};
        res *= rhs;
        return res;
    }

    template <typename T>
    std::ostream &operator<<(std::ostream &os, const Matrix<T> &rhs) {
        int nrows = rhs.GetRows();
        int ncolumns = rhs.GetColumns();

        for (int i = 0; i < nrows; ++i) {
            for (int j = 0; j < ncolumns; ++j) {
                os << rhs[i][j] << " ";
            }

            os << std::endl;
        }

        return os;
    }

    template <typename T>
    std::istream &operator>>(std::istream &is, const Matrix<T> &rhs) {
        int nrows = rhs.GetRows();
        int ncolumns = rhs.GetColumns();

        for (int i = 0; i < nrows; ++i) {
            for (int j = 0; j < ncolumns; ++j) {
                is >> rhs[i][j];
            }
        }

        return is;
    }

    template <typename T>
    Matrix<T>& Matrix<T>::RemoveRow(int n) & {
        assert(n < nrows_ && n >= 0);

        T* tmp_row = nullptr;

        delete[] data_[n];
        --nrows_;

        for (int i = n; i < nrows_; ++i) {
            tmp_row = data_[i];
            data_[i] = data_[i + 1];
            data_[i + 1] = tmp_row;
        }

        return *this;
    }

    template <typename T>
    Matrix<T> &Matrix<T>::RemoveColumn(int n) & {
        assert(n < ncolumns_ && n >= 0);

        --ncolumns_;
        T tmp_el;

        for (int i = 0; i < nrows_; ++i) {
            for (int j = n; j < ncolumns_; ++j) {
                tmp_el = data_[i][j];
                data_[i][j] = data_[i][j + 1];
                data_[i][j + 1] = tmp_el;
            }
        }

        return *this;
    }

    template <typename T>
    T Matrix<T>::GetMinor(int i, int j) const {
        assert(i >= 0 && j >= 0 && i < nrows_ && j < ncolumns_);
        Matrix<T> tmp{*this};

        tmp.RemoveRow(i);
        tmp.RemoveColumn(j);

        return tmp.Determinant();
    }

    template <typename T>
    Matrix<T>& Matrix<T>::Transpose() & {
        Matrix<T> res{GetColumns(), GetRows(), 0};

        for (int i = 0; i < nrows_; ++i) {
            for (int j = 0; j < ncolumns_; ++j) {
                res.data_[j][i] = data_[i][j];
            }
        }

        *this = res;
        return *this;
    }

    template <typename T>
    Matrix<T> Matrix<T>::Cofactors() const {
        Matrix<T> res{*this};

        for (int i = 0; i < nrows_; ++i) {
            for (int j = 0; j < ncolumns_; ++j) {
                res[i][j] = GetMinor(i, j);

                if ((i + j) % 2 != 0) {
                    res[i][j] = -res[i][j];
                }
            }
        }

        return res;
    }

    template <typename T>
    Matrix<T>& Matrix<T>::Inverse() & {
        float det = Determinant();

        *this = Cofactors();
        Transpose();
        *this *= (1 / det);

        return *this;
    }

    template <typename T>
    static Matrix<T> Zero(int n, int m) {
        return Matrix<T>{n, m, 0};
    }

    static Matrix<float> Rotation(int n, int i, int j, float angle) {
        Matrix<float> res = Matrix<float>::Eye(n);

        assert(i != j && i >= 0 && j >= 0);

        // Ensure that i < j
        if (i > j) {
            int tmp = i;
            i = j;
            j = tmp;
        }

        res[i][i] = std::cos(angle);
        res[i][j] = -std::sin(angle);
        res[j][i] = std::sin(angle);
        res[j][j] = std::cos(angle);

        return res;
    }

    template <typename T>
    Matrix<T>& Matrix<T>::SwapRows(int i, int j) & {
        T* tmp_row = data_[i];
        data_[i] = data_[j];
        data_[j] = tmp_row;

        return *this;
    }

    template <typename T>
    Matrix<T>& Matrix<T>::SubRows(int i, int j, T j_k) & {
        assert(i >= 0 && j >= 0 && i < nrows_ && j < ncolumns_);

        for (int m = 0; m < ncolumns_; ++m) {
            data_[i][m] -= (j_k * data_[j][m]);
            if (std::abs(data_[i][m]) < eps)
                data_[i][m] = 0;
        }

        return *this;
    }

    template <typename T>
    int Matrix<T>::ColumnMax(int j) const {
        int max = j;

        for (int i = j; i < nrows_; ++i) {
            if (std::abs(data_[i][j]) > std::abs(data_[max][j])) {
                max = i;
            }
        }

        return max;
    }
}
