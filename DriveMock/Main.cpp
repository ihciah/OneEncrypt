#include <iostream>
#include "FileTest.h"

using namespace std;

int main() {
	cout << "[DriveMock] DriveMock started." << endl;
	auto fileTest = FileTest();
	fileTest.ReadWriteTest("unenc/file.txt", "enc/file.txt");
	fileTest.ReadWriteTest("enc/file.txt", "unenc/filew.txt");

	//fileTest.ReadWriteTestMultipleTimes();
	cout << "[DriveMock] Read write finished." << endl;
	system("pause");
}