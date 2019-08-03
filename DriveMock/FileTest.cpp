#include "FileTest.h"

using namespace std;

void FileTest::ReadTest() {
	char data[101];

	ifstream infile;
	infile.open("testout.txt");
	infile.getline(data, 100);
	infile.close();
	cout << data << endl;
	cout << "Read finished" << endl;
}

void FileTest::WriteTest() {
	ofstream outfile;
	outfile.open("testout.txt");
	outfile << "This is a test!" << endl;
	outfile.close();
}
