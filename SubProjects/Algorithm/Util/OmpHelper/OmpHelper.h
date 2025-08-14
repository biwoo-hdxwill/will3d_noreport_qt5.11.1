#include "../Core/util_global.h"
#include <omp.h>
#include <vector>
class UTIL_EXPORT OmpHelper{
public:
	static void getThreadWorkSizeAndInterver(
		std::vector<int>& ompThreadWorkSize,
		std::vector<int>& ompThreadWorkInterver,
		int targetWorkSize,
		int nThreads
		);

	static void getThreadWorkInterver(
		std::vector<int>& ompThreadWorkInterver,
		const std::vector<int>& ompThreadWorkSize,
		int nThreads
		);

};
