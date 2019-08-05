#include "FileTest.h"

using namespace std;

void FileTest::ReadWriteTest() {
	char data[101];

	ifstream infile;
	infile.open("Atestout.txt");
	infile.getline(data, 100);
	infile.close();
	//cout << data << endl;
	cout << "Read finished" << endl;

	ofstream outfile;
	outfile.open("Btestout.txt");
	outfile << data << endl;
	outfile.close();
}

