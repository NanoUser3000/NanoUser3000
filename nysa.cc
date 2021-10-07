#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <map>
#include <set>
#include <vector>
#include <cstdint>
#include <cassert>

enum GateType {
	XOR, OR, NOR, AND, NAND, NOT,
	NULLGATE, 	//specjalna wartość, np. linia jest błędna -- nie zawiera żadnej bramki
};

using gate_t = std::pair<GateType, std::vector<int32_t>>;	//rodzaj bramki i sygnały wejściowe
using circuit_t = std::map<int32_t, gate_t>;	//klucze to sygnały wyjściowe, wartości to bramki, z których wychodzą

enum GateType GateTypeFromString(std::string s) {
	if (s == "XOR") {
		return XOR;
	}
	else if (s == "NOT") {
		return NOT;
	}
	else if (s == "AND") {
		return AND;
	}
	else if (s == "NAND") {
		return NAND;
	}
	else if (s == "OR") {
		return OR;
	}
	else if (s == "NOR") {
		return NOR;
	}
	else {
		return NULLGATE;
	}
}

bool goodGate(gate_t gate) {
	switch(gate.first) {
		case NULLGATE:
			//assert(false);	//!?
			return false;//!whatever
		case NOT:
			return gate.second.size() == 1;
		case XOR:
			return gate.second.size() == 2;
		default:
			return gate.second.size() >= 2;
	}
}

void errorLine( int32_t lineNo, std::string line) {
	std::cout << "Error in line " << lineNo << ": " << line << std::endl;
}

void errorMulOutput(int32_t lineNo, int32_t signalNo) {
	std::cout << "Error in line " << lineNo << ": "
		<< "signal " << signalNo << " is assigned to multiple outputs" << std::endl;	
}

//!ok że globalne?
std::regex correctRegexXOR("^[[:space:]]*XOR([[:space:]]+0*[1-9]([0-9]){0,8}){3}[[:space:]]*$");//3 liczby
std::regex correctRegexNOT("^[[:space:]]*NOT([[:space:]]+0*[1-9]([0-9]){0,8}){2}[[:space:]]*$");//2 liczby
std::regex correctRegexRest("^[[:space:]]*(AND|OR|NAND|NOR)([[:space:]]+0*[1-9]([0-9]){0,8}){3,}[[:space:]]*$");//co najm. 3 liczby

/**
 * Sprawdza też, czy liczby są z zakresu 1, 999999999
 */
GateType whatType(std::string line) {
	//! tu regexy?
	if (std::regex_match(line, correctRegexXOR)) {
		return XOR;
	}
	if (std::regex_match(line, correctRegexNOT)) {
		return NOT;
	}
	if (std:: regex_match(line, correctRegexRest)) {
		std::string type;
		std::stringstream(line) >> type;	//nie zmienia line
		return GateTypeFromString(type);
	}
	return NULLGATE;
}

/**
 * Wczytuje sygnał z linii do obwodu (o ile linia jest poprawna składniowo).
 * Sparametryzowana kontekstem wywołania.
 */
void addSignal(circuit_t circuit, std::string line, int32_t lineNo, bool& error) {
	GateType type = whatType(line);
	switch (type) {
		case NULLGATE:
		{
			errorLine(lineNo, line);
			error = true;
			break;
		}
		default:
		{
			std::stringstream str(line);
			std::string gType;
			str >> gType;
			assert(GateTypeFromString(gType) == type);

			gate_t gate;
			gate.first = type;

			int32_t outSignal;
			str >> outSignal;
			if (circuit.find(outSignal) != circuit.end()) {
				errorMulOutput(lineNo, outSignal);
				error = true;
			}

			int32_t inSignal;
			while (str >> inSignal) {
				gate.second.push_back(inSignal);
			}
			assert(goodGate(gate));

			str >> std::ws;
			assert(str.eof());
			
			circuit[outSignal] = gate;

			std::cout << "tak, wczytano " << gType << std::endl;//!//D

			break;
		}
	}
}

int main(void) {

	bool error = false;
	circuit_t circuit;	//struktura reprezentująca obwód

	std::string line;
	int32_t lineNo = 0;
	while (std::getline(std::cin, line)) {
		++lineNo;
		addSignal(circuit, line, lineNo, error);	//error może być zmieniony
	}

	//obwód utworzony (jako zmienna circuit)
	if (error) {
		std::cout << "error" << std::endl;//!//D
		return 0;
	}

	/*poprawny obwód (jeśli chodzi o składnię danych
	i to, że każdy sygnał może być maks. jednym wyjściem) */

	std::set<int32_t> inSignals;//do uzupełnienia podczas szukania cykli
	
	//w funkcji rekur. szukającej cykli:
	/*
	if(circuit.find(n) == circuit.end()) {
		inSignals.insert(n);
		return true;	//jest ok jeśli chodzi o cykle -- doszliśmy do sygnału wejściowego
	}
	else {
		//rekurencja szukająca cykli:
		coś_tam_coś(map[n])
	}
	*/

	return 0;
}