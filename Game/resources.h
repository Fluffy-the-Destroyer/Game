#pragma once
#include <string>

extern bool g_useCustomData;

class resource {
private:
	std::string s = "MISSING_NAME";
	std::string p = "MISSING_NAME";
public:
	void loadFromFile(std::string resource, bool custom = g_useCustomData);
	std::string singular() { return s; }
	std::string plural() { return p; }
	//Returns the singular form with a capital
	std::string Singular();
	//Returns the plural form with a capital
	std::string Plural();
	//Loads names from save
	void loadSave(std::ifstream* saveFile);
};