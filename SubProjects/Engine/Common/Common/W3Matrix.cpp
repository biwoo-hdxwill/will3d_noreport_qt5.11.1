#include "W3Matrix.h"
#include <assert.h>
#include <math.h>
#include "W3Vector.h"

namespace {
const double kEPS = 1.0e-4;
inline double SQR(const double& value) noexcept { return value * value; }
inline double SIGN(const double& a, const double& b) {
  return b >= 0.0 ? fabs(a) : -fabs(a);
}
}  // namespace

CW3Matrix::CW3Matrix() {}

CW3Matrix::CW3Matrix(int x, int y) {
  m_on = m_n = x;
  m_m = y;
  m_pv = new CW3Vector[m_n];

  for (int i = 0; i < m_n; i++) m_pv[i].SetSize(m_m);
}

CW3Matrix::~CW3Matrix() {
  if (m_pv) delete[] m_pv;
}

void CW3Matrix::SetValue(double** d) {
  for (int i = 0; i < m_n; i++)
    for (int j = 0; i < m_m; j++) d[i][j] = m_pv[i][j];
}

void CW3Matrix::SetValue(const int& row, const int& column,
                         const double& value) {
  this->m_pv[row].SetValue(column, value);
}

void CW3Matrix::SetSize(int x, int y) {
  if (m_on < x) {
    if (m_on > 0) delete[] m_pv;
    m_pv = new CW3Vector[x];
    m_on = x;
    for (int i = 0; i < x; i++) m_pv[i].SetSize(y);
  } else if (m_m < y)
    for (int i = 0; i < m_on; i++) m_pv[i].SetSize(y);
  m_n = x;
  m_m = y;
}

void CW3Matrix::Assign(CW3Matrix const& a) {
  SetSize(a.Row(), a.Column());

  for (int i = 0; i < a.Row(); i++) {
    for (int j = 0; j < a.Column(); j++) m_pv[i][j] = a[i][j];
  }
}

CW3Vector& CW3Matrix::operator[](const int& i) const { return m_pv[i]; }

void CW3Matrix::LUdecompose(int* index) {
  assert(this->Row() == this->Column());

  int n = this->Row();
  int i, j, k, imax;
  double big, dum, sum, temp;

  CW3Vector vv;
  vv.SetSize(n);

  for (i = 0; i < n; i++) {
    big = 0.0;
    for (j = 0; j < n; j++)
      if ((temp = fabs(m_pv[i][j])) > big) big = temp;

    if (big == 0.0) {
      // Singular matrix in routine LUdecompose
      assert(0);
    }

    vv[i] = 1.0 / big;
  }

  for (j = 0; j < n; j++) {
    for (i = 0; i < j; i++) {
      sum = m_pv[i][j];
      for (k = 0; k < i; k++) sum -= m_pv[i][k] * m_pv[k][j];
      m_pv[i][j] = sum;
    }

    big = 0.0;
    for (i = j; i < n; i++) {
      sum = m_pv[i][j];
      for (k = 0; k < j; k++) sum -= m_pv[i][k] * m_pv[k][j];
      m_pv[i][j] = sum;
      if ((dum = vv[i] * fabs(sum)) >= big) {
        big = dum;
        imax = i;
      }
    }

    if (j != imax) {
      for (k = 0; k < n; k++) {
        dum = m_pv[imax][k];
        m_pv[imax][k] = m_pv[j][k];
        m_pv[j][k] = dum;
      }
      vv[imax] = vv[j];
    }

    index[j] = imax;
    if (m_pv[j][j] == 0.0) m_pv[j][j] = kEPS;

    if (j != n) {
      dum = 1.0 / m_pv[j][j];
      for (i = j + 1; i < n; i++) m_pv[i][j] *= dum;
    }
  }
}

void CW3Matrix::LUsubstitute(int* index, CW3Vector& b) {
  assert(this->Row() == this->Column());

  int n = this->Row();
  int i, ii = -1, ip, j;
  double sum;

  for (i = 0; i < n; i++) {
    ip = index[i];
    sum = b[ip];
    b[ip] = b[i];

    if (ii > -1)
      for (j = ii; j < i; j++) sum -= m_pv[i][j] * b[j];
    else if (sum)
      ii = i;

    b[i] = sum;
  }

  for (i = n - 1; i >= 0; i--) {
    sum = b[i];
    for (j = i + 1; j < n; j++) sum -= m_pv[i][j] * b[j];
    b[i] = sum / m_pv[i][i];
  }
}
