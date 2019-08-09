#pragma once
#include <fstream>
#include <iostream>
#include <string>

class FileTest
{
private:
	std::string targetFile;
public:
	FileTest():targetFile("testout.txt"){}
	void ReadWriteTest();
	void ReadWriteTestMultipleTimes();
};

