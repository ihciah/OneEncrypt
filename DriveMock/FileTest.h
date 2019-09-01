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
	void ReadWriteTest(std::string srcPath, std::string dstPath);
	void ReadWriteTestMultipleTimes(std::string srcPath, std::string dstPath);
};

