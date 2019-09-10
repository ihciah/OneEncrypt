#pragma once
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>

class Logger {
private:
	std::unique_ptr<std::wofstream> ws;
public:
	Logger(const std::string);
	Logger(const char*);
	~Logger();
	void Print(const std::wstring);
	void Print(const std::string);
};

std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const std::string& s);
std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const std::wstring& s);
std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const char* s);
std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const wchar_t* s);
