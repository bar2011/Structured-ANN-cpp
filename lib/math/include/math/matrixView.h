#pragma once

#include "matrixBase.h"
#include "vector.h"

#include <vector>

namespace Math {

template <typename T> class Matrix;

// Class which mimics Math::Matrix class, but holds a reference to a data vector
// instead of the data itself. Hence, it has no ownership of the data it holds.
template <typename T> class MatrixView : public MatrixBase<T> {
public:
  // Create MatrixView which points at nothing
  MatrixView() = default;

  MatrixView(const MatrixView &other) = default;
  MatrixView(MatrixView &&other) = default;

  MatrixView &operator=(const MatrixView &other) = default;
  MatrixView &operator=(MatrixView &&other) = default;

  // Single item access - NO BOUNDS CHECKING
  const T &operator[](const size_t row, const size_t col) const;
  // Single item access - WITH BOUNDS CHECKING
  const T &at(const size_t row, const size_t col) const;

  // Reshapes matrix view to given dimensions. Returns *this.
  // Throws if given (rows * cols) is not equal to current (rows * cols).
  MatrixView &reshape(const size_t rows, const size_t cols);

  // Getters
  size_t rows() const { return m_rows; }
  size_t cols() const { return m_cols; }

  // Return entire underlying data. Not necessarily from the start of MatrixView
  const std::vector<T> &data() const { return *m_data; };

  // Returns the index of the biggest value in the matrix
  // format: (row, col)
  std::pair<size_t, size_t> argmax() const;

  // Returns a vector containing the index of the biggest value in each row
  Math::Vector<size_t> argmaxRow() const;

  // Returns a vector containing the index of the biggest value in each column
  Math::Vector<size_t> argmaxCol() const;

  // Returns a view of the entire matrix
  const Math::MatrixView<T> view() const;

  // Returns a view of a range of rows from the matrix.
  // Includes rows in the range [startRow, endRow), i.e., startRow is inclusive,
  // endRow is exclusive. The view includes all columns in each row.
  // Throws if endRow > row count or startRow >= endRow.
  const MatrixView<T> view(size_t startRow, size_t endRow) const;

  // Creates a new transposed matrix. Returns the transposed one.
  Matrix<T> transpose(size_t chunkSize = 4,
                      std::optional<bool> parallelize = std::nullopt) const;

  virtual Math::VectorView<T> asVector();

  friend Matrix<T>;

private:
  MatrixView(size_t start, size_t rows, size_t cols, const std::vector<T> &data)
      : m_data{&data}, m_start{start}, m_rows{rows}, m_cols{cols} {}

  const std::vector<T> *m_data{nullptr};
  size_t m_start{};
  size_t m_rows{};
  size_t m_cols{};
};
} // namespace Math

#include "matrixView.tpp"
