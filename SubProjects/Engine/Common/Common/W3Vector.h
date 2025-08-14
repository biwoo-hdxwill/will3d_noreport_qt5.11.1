#pragma once
#include <assert.h>
#include "common_global.h"
class CW3Matrix;

/* Class declaration **********************************************************/
class COMMON_EXPORT CW3Vector {
 public:
  CW3Vector();
  CW3Vector(int);
  ~CW3Vector();

  void SetValue(double*);
  inline void SetValue(const int& i, const double& d) { m_pv[i] = d; }

  inline double& operator[](const int& i) const { return m_pv[i]; }
  inline const double* const ConstData() const { return m_pv; }

  inline const int& size() const noexcept { return size_; }
  void SetSize(int);

  // LU Decomposition
  void Solve(CW3Matrix const&, CW3Vector const&);

 private:
   void Assign(const CW3Vector& v);

private:
  int size_ = 0, m_on = 0;
  double* m_pv = nullptr;
  int* m_pnIndex = nullptr;
};
