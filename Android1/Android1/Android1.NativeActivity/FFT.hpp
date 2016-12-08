#pragma once
typedef std::complex<double> Complex;
typedef unsigned int uint;


class FftBox {
private:

	Complex* one;
	Complex* ione;
	Complex *array;
	size_t size; // arrayのサイズ
	size_t use; // コンストラクタで与えたサイズ
	double* Spectol;
	bool verbose; // 配列ダンプのモード指定に使う
	inline size_t nextPow2(size_t s) {
		size_t n = 1;
		while (n < s)
			n <<= 1;
		return n;
	}
	inline void bitReverse() {
		uint k, b, a;
		for (uint i = 0; i < size; i++) {
			k = 0;
			b = size >> 1;
			a = 1;
			while (b >= a) {
				if (b & i) k |= a;
				if (a & i) k |= b;
				b >>= 1;
				a <<= 1;
			}
			if (i < k)
				swap(array[i], array[k]);
		}
	}
public:
	FftBox(size_t s) : use(s), size(nextPow2(s)), verbose(false) {
		array = new Complex[size];
		one = new std::complex<double>(1.0, 0.0);
		ione = new std::complex<double>(0.0, 1.0);
		Spectol = new double[size];
	};
	~FftBox() {
		delete[] array;
		delete[] Spectol;
		delete[] one;
		delete[] ione;
	};

	Complex& operator[](int index)const {
		if (index < 0)
			return array[use - index];
		return array[index];
	};
	inline double* GetSpectol() {
		uint end = verbose ? size : use;
		for (uint i = 0; i < end; i++)
			Spectol[i] =  std::abs(array[i]);
		return Spectol;
	}
	bool setVerbose(bool b) {
		return verbose = b;
	};
	inline void dump() {
		uint end = verbose ? size : use;
		for (uint i = 0; i < end; i++)
			std::cout << array[i] << " ";
		std::cout << std::endl;
	}

	inline void fft(bool isReverse = false){
		bitReverse();
		size_t m = 2;
		Complex w, ww, t;

		while (m <= size) {
			double arg = -2.0 * M_PI / m;
			w = Complex(cos(arg), sin(arg));
			if (isReverse)
				w = (*one) / w; //-1乗 -(-2.0*PI/size) = 2.0*PI/size

			for (uint i = 0; i < size; i += m) {
				ww = 1.0;
				for (uint j = 0; j < m / 2; j++) {
					int a = i + j;
					int b = i + j + m / 2;

					t = ww * array[b];

					array[b] = array[a] - t;
					array[a] = array[a] + t;

					ww *= w;
				}
			}
			m *= 2;
		}
	}
	inline void ifft() {
		fft(true);
		double s = (double)size;
		for (uint i = 0; i < size; i++)
			array[i] /= s;
	}
};
