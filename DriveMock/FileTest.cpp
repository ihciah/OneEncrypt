#include "FileTest.h"

using namespace std;

void FileTest::ReadWriteTest() {
	std::ifstream input("Atestout.txt", std::ios::binary);
	std::ofstream output("Btestout.txt", std::ios::binary);

	std::copy(
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>(),
		std::ostreambuf_iterator<char>(output));
	input.close();
	output.close();
}

