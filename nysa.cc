#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <map>
#include <set>
#include <vector>
#include <cstdint>
#include <cassert>
#include <math.h>

enum GateType {
	XOR, OR, NOR, AND, NAND, NOT,
	NULLGATE, 	//specjalna wartość, np. linia jest błędna -- nie zawiera żadnej bramki
};

using gate_t = std::pair<GateType, std::vector<int32_t>>;	//rodzaj bramki i sygnały wejściowe
using circuit_t = std::map<int32_t, gate_t>;	//klucze to sygnały wyjściowe, wartości to bramki, z których wychodzą
using signalCombinations = std::vector<std::vector<bool>>; //struktura do przechowywania kombinacji sygnałów wejsicowych
using mapIntBool = std::map<int32_t,bool>; // lepsza nazwa by sie przydala, oczy mnie bola jak patrze na to std::map<int32_t,bool> wszedzie
using pairIntBool = std::pair<int32_t,bool>; // tutaj to samo mozna usunac to pod koniec jak sie niczego ladniejszego nie wymysli

/**
 * Funkcja dodaje binarnie 1 do combinations
 */
void nextCombination(std::vector<bool> &combination) {
	bool carry = true;
	for (int i = combination.size()-1; i >= 0; i--) {
		if (combination[i] && carry) {
			combination[i] = false;
		}
		else {
			combination[i] = true;
			return;
		}
	}
}

bool andGate(std::vector<bool> input) {
	bool result = input[0];
	for (size_t i = 1; i < input.size(); ++i)
		result &= input[i];
	return result;
}

bool orGate(std::vector<bool> input) {
	bool result = input[0];
	for (size_t i = 1; i < input.size(); ++i)
		result |= input[i];
	return result;
}

bool calcGate(GateType type, std::vector<bool> input) {
	if (type == XOR)
		return input[0] ^ input[1];
	else if (type == OR)
		return orGate(input);
	else if (type == NOR)
		return !orGate(input);
	else if (type == AND)
		return andGate(input);
	else if (type == NAND)
		return !andGate(input);
	else if (type == NOT)
		return (!input[0]);
	return false;
}

void calcWire(int32_t wire, circuit_t &circuit, std::map<int32_t,int32_t> &values) {
	if (values.at(wire) != -1)
		return;
	else {
		// liczymy wszystkie potrzebne przewody do wyliczenia wire
		std::vector<bool> inGateValues; // wartosci przewodow wchodzacych do bramki z ktorej wychodzi wire
		for (int32_t w : circuit.at(wire).second) {
			calcWire(w, circuit, values);
			inGateValues.push_back(values.at(w));
		}
		if (wire == 7){
			/*
			std::cout <<"dupa";
			for (size_t i = 0; i < inGateValues.size(); i++) {
				std::cout << inGateValues[i]<< std::endl;
			}
			*/
		}
		values[wire] = calcGate(circuit.at(wire).first,inGateValues);
	}
}

void calculateTruthTable(circuit_t circuit, std::set<int32_t> &inSignals) {
	std::map<int32_t,int32_t> values;
	std::vector<bool> currentInput(inSignals.size(),false);	//wartosci dla przewodow wejsiowych

	//dodanie do mapy przewodow wejsciowych
	size_t index = 0;
	for (std::set<int32_t>::iterator it = inSignals.begin(); it != inSignals.end(); ++it)  {
		values[*it] = currentInput[index];
		index++;
	}

	// dodanie do mapy reszty przewodow
	for (circuit_t::iterator it = circuit.begin(); it != circuit.end(); ++it) 
		values[it->first] = -1;

	//petla wykonuje sie 2^(liczba przewodow wejsciowych) razy aby policzyc tabele prawdy dla kazdej kombinacji 
	for (int i = 0; i < pow(2,inSignals.size()); ++i) {
		for (circuit_t::iterator it = circuit.begin(); it != circuit.end(); ++it) {
			calcWire(it->first,circuit,values);
		}
		//po obliczeniu wypisujemy warosci
		for (std::map<int32_t,int32_t>::iterator it = values.begin(); it != values.end(); ++it) {
			std::cout << it->second;
		}
		std::cout << std::endl;
		
		nextCombination(currentInput);
		size_t index = 0;
		for (std::set<int32_t>::iterator it = inSignals.begin(); it != inSignals.end(); ++it)  {
			values[*it] = currentInput[index];
			index++;
		}
		for (circuit_t::iterator it = circuit.begin(); it != circuit.end(); ++it) 
			values[it->first] = -1;
	}
}

void dfsCycleDetection(int32_t wire, circuit_t &circuit, mapIntBool &visited, mapIntBool &dfsVisited, bool &cycleDetected, std::set<int32_t> &signals) {
	visited.at(wire) = true;
	dfsVisited.at(wire) = true;
	for (int32_t w : circuit.at(wire).second) {
		circuit_t::iterator it = circuit.find(w);
		if (it == circuit.end()) {
			signals.insert(w);
		}
		else {
			if (!visited.at(w)) 
				dfsCycleDetection(w, circuit, visited, dfsVisited, cycleDetected, signals);
			else if (dfsVisited.at(w))
				cycleDetected = true;
		}
	}
	dfsVisited[wire] = false;
	return;
}

/**
 * Funkcja przygotowuje wszystkie potrzebne dane do wykrywania cykli czyli dwie mapy do oznaczania odwiedzonych przewodow
 */
void cycleDetection(circuit_t &circuit, std::set<int32_t> &signals) {
	mapIntBool visited;
	mapIntBool dfsVisited;
	for (circuit_t::iterator it = circuit.begin(); it != circuit.end(); ++it) {
		visited.insert(pairIntBool(it->first,false));
		dfsVisited.insert(pairIntBool(it->first,false));
	}

	bool cycleDetected = false;

	for (mapIntBool::iterator it = visited.begin(); it != visited.end(); ++it) {
		if (it->second == false) 
			dfsCycleDetection(it->first, circuit, visited, dfsVisited, cycleDetected, signals);
	}

	if (cycleDetected) {
		std::cout << "Error: sequential logic analysis has not yet been implemented."<<std::endl;
		return;
	}
}
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
void addSignal(circuit_t &circuit, std::string line, int32_t lineNo, bool& error) {
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
			
			//circuit.insert(std::pair<int32_t,gate_t>(outSignal,gate));
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
	std::set<int32_t> signals;

	cycleDetection(circuit, signals);
	
	calculateTruthTable(circuit, signals);
	
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
