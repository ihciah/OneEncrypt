#include "Logger.h"

using namespace std;

Logger::Logger(string filename) {
	ws = make_unique<wofstream>(wofstream(filename));
	locale utf8_locale(ws->getloc(), new codecvt_utf8<wchar_t>);
	ws->imbue(utf8_locale);
	*ws << L"[Logger]Logger started." << endl;
}

Logger::Logger(char* filename) {
	Logger(string(filename));
}

Logger::~Logger() {
	ws->close();
	ws.~unique_ptr();
}

void Logger::Print(wstring logString) {
	*ws << logString;
	ws->flush();
}

void Logger::Print(string logString) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	*ws << converter.from_bytes(logString);
	ws->flush();
}

std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const string& s){
	out->Print(s);
	return out;
}

std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const wstring& s) {
	out->Print(s);
	return out;
}

std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const char* s) {
	out->Print(string(s));
	return out;
}

std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const wchar_t* s) {
	out->Print(wstring(s));
	return out;
}
