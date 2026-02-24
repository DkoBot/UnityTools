#include "InfoMesseng.h"
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

void InfoMesseng::ColorPrint(const char* iNFOtext, const char* text, int ColorId) {
	// 获取当前时间
	auto now = chrono::system_clock::now();
	auto time_t = chrono::system_clock::to_time_t(now);
	tm tm_buf;
	localtime_s(&tm_buf, &time_t);
	stringstream ss;
	ss << put_time(&tm_buf, "%H:%M:%S");
	string timeStr = ss.str();

	// 绿色消息
	if (ColorId == 0) {
		cout << fg::magenta << "[" << timeStr << "] " << style::reset
			 << style::bold << fg::green << "[" << iNFOtext << "] " << style::reset << text << endl;
	}
	// 红色消息
	else if (ColorId == 1) {
		cout << fg::magenta << "[" << timeStr << "] " << style::reset
			 << style::bold << fg::red << "[" << iNFOtext << "] " << style::reset << text << endl;
	}
	// 黄色消息
	else if (ColorId == 2) {
		cout << fg::magenta << "[" << timeStr << "] " << style::reset
			 << style::bold << fg::yellow << "[" << iNFOtext << "] " << style::reset << text << endl;
	}
	// 蓝色消息
	else if (ColorId == 3) {
		cout << fg::magenta << "[" << timeStr << "] " << style::reset
			 << style::bold << fg::blue << "[" << iNFOtext << "] " << style::reset << text << endl;
	}
	// 白色消息
	else{
		cout << fg::magenta << "[" << timeStr << "] " << style::reset
			 << style::bold << fg::gray << "[" << iNFOtext << "] " << style::reset << text << endl;
	}
}