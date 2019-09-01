#include "FileTest.h"

using namespace std;

// Copy file in stream
void FileTest::ReadWriteTest(std::string srcPath, std::string dstPath) {
	std::ifstream input(srcPath, std::ios::binary);
	std::ofstream output(dstPath, std::ios::binary);

	auto src = std::istreambuf_iterator<char>(input);
	auto srcEnd = std::istreambuf_iterator<char>();
	auto dst = std::ostreambuf_iterator<char>(output);

	std::copy(src, srcEnd, dst);

	input.close();
	output.close();
}

// Copy file in multiple fread and fwrite
// However, manually fread does not mean ReadFile call
// So if testing multiple read and write, use a little big file
void FileTest::ReadWriteTestMultipleTimes(std::string srcPath, std::string dstPath) {
	FILE *ptr, *wptr;
	const unsigned int BUFFER_SIZE = 2048;
	unsigned char buffer[BUFFER_SIZE];

	fopen_s(&ptr, srcPath.c_str(), "rb");
	fopen_s(&wptr, dstPath.c_str(), "wb");
	while (true) {
		int x = fread_s(buffer, sizeof(buffer), 1, sizeof(buffer), ptr);
		if (x <= 0) break;
		cout << x << endl;
		fwrite(buffer, x, 1, wptr);
	}

	fclose(ptr);
	fclose(wptr);
}
