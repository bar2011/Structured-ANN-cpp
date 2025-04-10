// DO NOT INCLUDE THIS FILE - include matrix.h instead
#pragma once

#include "matrix.h"
#include <algorithm>
#include <utility>

namespace Math {

template <typename T>
Matrix<T>::Matrix(const size_t rows, const size_t cols, std::function<T()> gen)
    : m_data(rows * cols), m_rows{rows}, m_cols{cols} {
  std::generate_n(m_data.begin(), rows * cols, gen);
}

template <typename T>
Matrix<T>::Matrix(const size_t rows, const size_t cols, const T *const data)
    : m_data(rows * cols), m_rows{rows}, m_cols{cols} {
  std::copy_n(data, rows * cols, m_data.begin());
}

template <typename T>
Matrix<T>::Matrix(const size_t rows, const size_t cols)
    : m_data(rows * cols), m_rows{rows}, m_cols{cols} {};

template <typename T>
Matrix<T>::Matrix(const Matrix &other)
    : m_data(other.m_data.size()), m_rows{other.m_rows}, m_cols{other.m_cols} {
  std::copy_n(other.m_data.begin(), m_rows * m_cols, m_data.begin());
}

template <typename T>
Matrix<T>::Matrix(Matrix &&other) noexcept
    : m_data{std::move(other.m_data)}, m_rows{other.m_rows},
      m_cols{other.m_cols} {
  other.m_rows = 0;
  other.m_cols = 0;
}

template <typename T> Matrix<T> &Matrix<T>::operator=(const Matrix &other) {
  if (&other != this) {
    m_rows = other.m_rows;
    m_cols = other.m_cols;
    m_data.resize(m_rows * m_cols);
    std::copy_n(other.m_data.begin(), m_rows * m_cols, m_data.begin());
  }
  return *this;
}

template <typename T> Matrix<T> &Matrix<T>::operator=(Matrix &&other) noexcept {
  if (&other != this) {
    m_data = std::move(other.m_data);
    m_rows = other.m_rows;
    m_cols = other.m_cols;
    other.m_rows = 0;
    other.m_cols = 0;
  }
  return *this;
}

template <typename T>
T &Matrix<T>::operator[](const size_t row, const size_t col) {
  if (row >= m_rows)
    throw Math::Exception{
        "Math::Matrix<T>::operator[](const size_t, const size_t)",
        "Invalid row number: out of bounds"};
  if (col >= m_cols)
    throw Math::Exception{
        "Math::Matrix<T>::operator[](const size_t, const size_t)",
        "Invalid column number: out of bounds"};

  return m_data[row * m_cols + col];
}

template <typename T>
const T &Matrix<T>::operator[](const size_t row, const size_t col) const {
  if (row >= m_rows)
    throw Math::Exception{
        "Math::Matrix<T>::operator[](const size_t, const size_t)",
        "Invalid row number: out of bounds"};
  if (col >= m_cols)
    throw Math::Exception{
        "Math::Matrix<T>::operator[](const size_t, const size_t)",
        "Invalid column number: out of bounds"};

  return m_data[row * m_cols + col];
}

template <typename T>
Matrix<T> operator+(const Matrix<T> &m, const Vector<T> &v) {
  if (m.cols() == v.size()) { // row wise addition
    Matrix<T> result{m.rows(), m.cols()};
    for (size_t i{}; i < m.rows(); ++i)
      for (size_t j{}; j < m.cols(); ++j)
        result[i, j] = m[i, j] + v[j];

    return result;
  }

  if (m.rows() == v.size()) { // column wise addition
    Matrix<T> result{m.rows(), m.cols()};
    for (size_t i{}; i < m.rows(); ++i)
      for (size_t j{}; j < m.cols(); ++j)
        result[i, j] = m[i, j] + v[i];

    return result;
  }

  throw Math::Exception{
      "Math::operator+(const Matrix<T>&, const Vector<T>&)",
      "Can't add matrix and vector where their sizes don't match for row or "
      "column wise addition"};
}

template <typename T>
void Matrix<T>::reshape(const size_t rows, const size_t cols) {
  if (rows * cols != m_rows * m_cols)
    throw Math::Exception{
        "Math::Matrix<T>::reshape(const size_t, const size_t)",
        "Can't reshape a matrix where the given dimensions don't give the same "
        "number of values as the previous dimensions"};

  m_rows = rows;
  m_cols = cols;
};
}; // namespace Math
