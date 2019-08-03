#include <iostream>
#include "FileTest.h"

using namespace std;

int main() {
	cout << "DriveMock started." << endl;
	auto fileTest = FileTest();
	fileTest.WriteTest();
	cout << "write finished." << endl;
	fileTest.ReadTest();
	cout << "read started." << endl;
	system("pause");
}