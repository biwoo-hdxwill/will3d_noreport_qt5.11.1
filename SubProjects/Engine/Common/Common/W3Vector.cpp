#include "W3Vector.h"
#include <assert.h>
#include "W3Matrix.h"

CW3Vector::CW3Vector() {}

CW3Vector::CW3Vector(int x) {
  m_on = size_ = x;
  if (m_on > 0) m_pv = new double[size_];
}

CW3Vector::~CW3Vector() {
  if (m_pv) delete[] m_pv;
  if (m_pnIndex) delete[] m_pnIndex;
}

void CW3Vector::SetValue(double* d) {
  std::memcpy(m_pv, d, sizeof(double)*size_);
}

void CW3Vector::SetSize(int x) {
  if (m_on < x) {
    if (m_on > 0) delete[] m_pv;
    m_pv = new double[x];
    m_on = x;
  }
  size_ = x;
}

void CW3Vector::Assign(const CW3Vector& a) {
  SetSize(a.size());
  std::memcpy(m_pv, a.ConstData(), sizeof(double)*a.size());
}

void CW3Vector::Solve(CW3Matrix const& a, CW3Vector const& b) {
  assert(a.Row() == a.Column());
  assert(a.Row() == b.size());

  Assign(b);

  if (m_pnIndex) delete[] m_pnIndex;
  m_pnIndex = new int[b.size()];

  CW3Matrix mat;
  mat.Assign(a);
  mat.LUdecompose(m_pnIndex);
  mat.LUsubstitute(m_pnIndex, *this);
}
