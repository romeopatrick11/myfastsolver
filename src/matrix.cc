#include "matrix.hpp"

#include <iostream>
#include <assert.h>
#include <math.h>   // for sqrt()
#include <stdlib.h> // for srand48_r(), lrand48_r() and drand48_r()
#include <time.h>

Vector::Vector() : nPart(-1), mRows(-1), generate_entry(true) {}

Vector::Vector(int N, bool generate) : nPart(-1), mRows(N), generate_entry(generate) {
  assert(N>0);
  if (generate_entry) {
    // allocate memory
    data.resize(mRows);
  }
}

int Vector::rows() const {return mRows;}

int Vector::num_partition() const {return nPart;}

long Vector::rand_seed(int i) const {
  assert( 0<=i && i<nPart );
  return seeds[i];
}

double Vector::norm() const {
  assert( generate_entry == true );
  double sum = 0.0;
  for (int i=0; i<mRows; i++)
    sum += data[i]*data[i];
  return sqrt(sum);
}

void Vector::rand(int nPart_) {
  assert( mRows%nPart == 0 );
  this->nPart = nPart_;
  struct drand48_data buffer;
  assert( srand48_r( time(NULL)+2, &buffer ) == 0 );
  for (int i=0; i<nPart; i++) {
    long seed;
    assert( lrand48_r(&buffer, &seed) == 0 );
    seeds.push_back( seed );
  }

  // generating random numbers
  if (generate_entry) {
    int count = 0;
    int colorSize = mRows / nPart;
    for (int i=0; i<nPart; i++) {
      assert( srand48_r( seeds[i], &buffer ) == 0 );
      for (int j=0; j<colorSize; j++) {
	assert( drand48_r(&buffer, &data[count]) == 0 );
	count++;
      }
    }
  }
}

double& Vector::operator[] (int i) {
  assert( generate_entry == true );
  assert( 0 <= i && i < mRows );
  return data[i];
}

double Vector::operator[] (int i) const {
  assert( generate_entry == true );
  assert( 0 <= i && i < mRows );
  return data[i];
}

Vector Vector::multiply(const Vector& other) {
  assert( this->generate_entry == true );
  assert( other.generate_entry == true );
  int N = this->rows();
  assert( N == other.rows() );
  Vector temp(N);
  for (int i=0; i<N; i++)
    temp[i] = data[i] * other[i];
  return temp;
}

void Vector::display(const std::string& name) const {
  std::cout << name << ":" << std::endl;
  if (nPart > 0) {
    std::cout << "seeds:" << std::endl;
    for (int i=0; i<nPart; i++)
      std::cout << seeds[i] << "\t";
    std::cout << std::endl
	      << "values:"
	      << std::endl;
  }
  if (generate_entry) {
    for (int j=0; j<mRows; j++)
      std::cout << data[j] << "\t";
    std::cout << std::endl;
  }
}

Vector operator + (const Vector& vec1, const Vector& vec2) {
  assert( vec1.generate_entry == true );
  assert( vec2.generate_entry == true );
  assert( vec1.rows() == vec2.rows() );
  Vector temp(vec1.rows());
  for (int i=0; i<vec1.rows(); i++)
    temp[i] = vec1[i] + vec2[i];
  return temp;
}

Vector operator - (const Vector& vec1, const Vector& vec2) {
  assert( vec1.generate_entry == true );
  assert( vec2.generate_entry == true );
  assert( vec1.rows() == vec2.rows() );
  Vector temp(vec1.rows());
  for (int i=0; i<vec1.rows(); i++)
    temp[i] = vec1[i] - vec2[i];
  return temp;
}

bool operator== (const Vector& vec1, const Vector& vec2) {  
  assert( vec1.generate_entry == true );
  assert( vec2.generate_entry == true );
  assert( vec1.rows() == vec2.rows() );
  for (int i=0; i<vec1.rows(); i++) {
    if ( fabs(vec1[i] - vec2[i]) > 1e-10 )
      return false;
  }
  return true;
}

bool operator!= (const Vector& vec1, const Vector& vec2) {
  return !(vec1 == vec2);
}

Vector operator * (const double alpha,  const Vector& vec) {
  assert( vec.generate_entry == true );
  Vector temp(vec.rows());
  for (int i=0; i<vec.rows(); i++)
    temp[i] = alpha * vec[i];
  return temp;
}

Matrix::Matrix() : nPart(-1), mRows(-1), mCols(-1), generate_entry(true) {}

Matrix::Matrix(int row, int col, bool generate)
  : nPart(-1), mRows(row), mCols(col), generate_entry(generate) {
  
  assert( mRows>0 && mCols>0 );
  if (generate_entry) {
    // allocate memory
    data.resize(mRows*mCols);
  }
}

int Matrix::rows() const {return mRows;}

int Matrix::cols() const {return mCols;}

int Matrix::num_partition() const {return nPart;}

void Matrix::rand(int nPart_) {
  this->nPart = nPart_;
  assert( mRows%nPart == 0 );
  struct drand48_data buffer;
  assert( srand48_r( time(NULL)+1, &buffer ) == 0 );
  for (int i=0; i<nPart; i++) {
    long seed;
    assert( lrand48_r(&buffer, &seed) == 0 );
    seeds.push_back( seed );
  }

  // generating random numbers
  if (generate_entry) {
    int count = 0;
    int colorSize = mRows / nPart;
    for (int i=0; i<nPart; i++) {
      assert( srand48_r( seeds[i], &buffer ) == 0 );
      for (int j=0; j<colorSize*mCols; j++) {
	assert( drand48_r(&buffer, &data[count]) == 0 );
	count++;
      }
    }
  }
}

long Matrix::rand_seed(int i) const {
  assert( 0<=i && i<nPart );
  return seeds[i];
}

double Matrix::operator() (int i, int j) const {
  assert( generate_entry == true );
  return data[i+j*mRows];
}

double& Matrix::operator() (int i, int j) {
  assert( generate_entry == true );
  return data[i+j*mRows];
}

Matrix Matrix::T() {
  assert( generate_entry == true );
  Matrix temp(mCols, mRows);
  for (int i=0; i<mRows; i++)
    for (int j=0; j<mCols; j++)
      temp(j, i) = (*this)(i, j);
  return temp;
}

Vector operator * (const Matrix& A, const Vector& b) {
  assert( A.generate_entry == true );
  assert( A.cols() == b.rows() );
  Vector temp(A.rows());
  for (int i=0; i<temp.rows(); i++) {
    temp[i] = 0.0;
    for (int j=0; j<A.cols(); j++)
      temp[i] += A(i,j) * b[j];
  }
  return temp;
}

Matrix operator + (const Matrix& mat1, const Matrix& mat2) {
  assert( mat1.generate_entry == true );
  assert( mat2.generate_entry == true );
  assert(mat1.rows() == mat2.rows());
  assert(mat1.cols() == mat2.cols());
  Matrix temp(mat1.rows(), mat1.cols());
  for (int i=0; i<temp.rows(); i++)
    for (int j=0; j<temp.cols(); j++)
      temp(i, j) = mat1(i, j) + mat2(i, j);
  return temp;
}

Matrix operator - (const Matrix& mat1, const Matrix& mat2) {
  assert( mat1.generate_entry == true );
  assert( mat2.generate_entry == true );
  assert(mat1.rows() == mat2.rows());
  assert(mat1.cols() == mat2.cols());
  Matrix temp(mat1.rows(), mat1.cols());
  for (int i=0; i<temp.rows(); i++)
    for (int j=0; j<temp.cols(); j++)
      temp(i, j) = mat1(i, j) - mat2(i, j);
  return temp;
}

bool operator== (const Matrix& mat1, const Matrix& mat2) {
  assert( mat1.generate_entry == true );
  assert( mat2.generate_entry == true );
  assert(mat1.rows() == mat2.rows());
  assert(mat1.cols() == mat2.cols());
  for (int i=0; i<mat1.rows(); i++)
    for (int j=0; j<mat1.cols(); j++)
      if ( fabs( mat1(i, j) - mat2(i, j) ) > 1.0e-10 )
	return false;
  return true;
}

bool operator!= (const Matrix& mat1, const Matrix& mat2) {
  return !(mat1 == mat2);
}

Matrix operator * (const double alpha,  const Matrix& mat) {
  assert( mat.generate_entry == true );
  Matrix temp(mat.rows(), mat.cols());
  for (int i=0; i<temp.rows(); i++)
    for (int j=0; j<temp.cols(); j++)
      temp(i, j) = alpha * mat(i, j);
  return temp;
}

// the fastest increasing dimension is displayed first
void Matrix::display(const std::string& name) const {
  std::cout << name << ":" << std::endl;
  if (nPart > 0) {
    std::cout << "seeds:" << std::endl;
    for (int i=0; i<nPart; i++)
      std::cout << seeds[i] << "\t";
    std::cout << std::endl
	      << "values:"
	      << std::endl;
  }
  if (generate_entry) {
    int count = 0;
    for (int i=0; i<mRows; i++) {
      for (int j=0; j<mCols; j++)
	std::cout << data[count++] << "\t";
      std::cout << std::endl;
    }
  }
}
