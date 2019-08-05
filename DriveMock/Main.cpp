#include <iostream>
#include "FileTest.h"

using namespace std;

int main() {
	cout << "DriveMock started." << endl;
	auto fileTest = FileTest();
	fileTest.ReadWriteTest();
	cout << "read write finished." << endl;
	system("pause");
}