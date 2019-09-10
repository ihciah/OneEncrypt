#include <iostream>
#include <Windows.h>
#include "FileTest.h"

using namespace std;

void PrintCurrentDirectory() {
	CHAR buffer[MAX_PATH + 1];
	GetCurrentDirectoryA(MAX_PATH + 1, buffer);
	cout << "[DriveMock] Runtime directory: " << buffer << endl;
}

int main() {
	cout << "[DriveMock] DriveMock started." << endl;
	PrintCurrentDirectory();

	auto fileTest = FileTest();
	fileTest.ReadWriteTest("unenc/file.txt", "enc/file.txt");
	fileTest.ReadWriteTest("enc/file.txt", "unenc/filew.txt");

	//fileTest.ReadWriteTestMultipleTimes();
	cout << "[DriveMock] Read write finished." << endl;
	system("pause");
}
