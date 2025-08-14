#pragma once
#include "common_global.h"

class CW3Vector;

class COMMON_EXPORT CW3Matrix {
 private:
  int m_n = 0, m_m = 0, m_on = 0;
  CW3Vector* m_pv = nullptr;

 public:
  // constructors
  CW3Matrix();
  CW3Matrix(int, int);
  ~CW3Matrix();

  void Assign(CW3Matrix const&);

  // inquiry functions
  void SetValue(double**);
  void SetValue(const int& row, const int& column,
                const double& value);

  const int& Row() const noexcept { return m_n; }
  const int& Column() const noexcept { return m_m; }
  void SetSize(int, int);

  void LUdecompose(int*);
  void LUsubstitute(int*, CW3Vector&);

private:
  CW3Vector& operator[](const int& i) const;
};
