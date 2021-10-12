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
#include <unordered_set>

enum GateType {
	XOR, OR, NOR, AND, NAND, NOT,
	NULLGATE, 	//specjalna wartość, np. linia jest błędna -- nie zawiera żadnej bramki
};

using gate_t = std::pair<GateType, std::vector<int32_t>>;	//rodzaj bramki i sygnały wejściowe
using circuit_t = std::map<int32_t, gate_t>;	//klucze to sygnały wyjściowe, wartości to bramki, z których wychodzą
using markingMap = std::map<int32_t,std::pair<int32_t,int32_t>>; //klucz to numer przewodu para to dwie wartości potrzebne do topoSort tempMark i permamentMark

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

bool xor_(bool a, bool b) {
	if ((a && b) || (!a && !b))
		return false;
	return true;
}

bool calcGate(GateType type, std::vector<bool> &input) {
	switch (type) {
	    case XOR:
    		return xor_(input[0],input[1]);
    	case OR:
    		return orGate(input);
    	case NOR:
    		return !orGate(input);
    	case AND:
    		return andGate(input);
    	case NAND:
    		return !andGate(input);
    	case NOT:
    		return (!input[0]);
    	default:
    	    return false;
    }
}

void calcWire(circuit_t &circuit, int32_t wire, std::map<int32_t,int32_t> &values) {
	std::vector<bool> inputSignals;
	//std::cout << wire;
	//std::cout <<":";
	for (int32_t x : circuit.at(wire).second)  {
		inputSignals.push_back(values[x]);
		//std::cout << values[x];
	}
	//std::cout << std::endl;

	values[wire] = calcGate(circuit.at(wire).first,inputSignals);
}

void runSimulation(circuit_t &circuit, std::vector<int32_t> &order, std::set<int32_t> &inSignals) {
	std::map<int32_t,int32_t> values; // 0,1 albo -1 jak nie ma
	std::vector<bool> currInput(inSignals.size(),false); // poczatakowe wartosci inputu

	for (int32_t inSignal : inSignals)
		values[inSignal] = -1;

	for (int32_t n : order)
		values[n] = -1;

	for (size_t i = 0; i < pow(2,inSignals.size()); i++) {
		size_t j = 0;
		for (auto it = inSignals.begin(); it != inSignals.end(); it++) {
			values[*it] = currInput[j];
			j++;
		}


		for (int32_t n : order)
			calcWire(circuit,n,values);

		for (auto it = values.begin(); it != values.end(); ++it)
			std::cout << it->second;
		std::cout << std::endl;

		nextCombination(currInput);
	}
}

void visit(int32_t n, circuit_t &circuit, markingMap &mark, std::vector<int32_t> &order, std::set<int32_t> &inSignals, bool &dag) {
	auto it = mark.find(n);
	if (it == mark.end()) {
		inSignals.insert(n);
		return;
	}
	// sprawdza temporary mark
	if (mark.at(n).first) {
		dag = false;
		return;
	}
	// sprawdza permament mark
	if (mark.at(n).second)
		return;
	
	mark.at(n).first = true;
	for (int32_t m : circuit.at(n).second)
		visit(m,circuit,mark,order,inSignals,dag);
	
	mark.at(n).first = false;
	mark.at(n).second = true;

	order.push_back(n);
}

void topoSort(circuit_t &circuit, std::vector<int32_t> &order, std::set<int32_t> &inSignals, bool &dag) {

	markingMap mark;

	for (auto it = circuit.begin(); it != circuit.end(); ++it)
		mark[it->first] = std::make_pair(false,false);

	for (auto it = mark.begin(); it != mark.end(); ++it) {
		if (!mark.at(it->first).second)
			visit(it->first,circuit, mark, order, inSignals, dag);
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
			//assert(false)
			return false;   //whatever
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
		<< "signal " << signalNo << " is assigned to multiple outputs." << std::endl;
}

/**
 * Sprawdza też, czy liczby są z zakresu 1, 999999999
 */
GateType whatType(std::string line) {
    std::regex correctRegexXOR("^[[:space:]]*XOR([[:space:]]+0*[1-9]([0-9]){0,8}){3}[[:space:]]*$");//3 liczby
    std::regex correctRegexNOT("^[[:space:]]*NOT([[:space:]]+0*[1-9]([0-9]){0,8}){2}[[:space:]]*$");//2 liczby
    std::regex correctRegexRest("^[[:space:]]*(AND|OR|NAND|NOR)([[:space:]]+0*[1-9]([0-9]){0,8}){3,}[[:space:]]*$");//co najm. 3 liczby

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

			circuit[outSignal] = gate;

			break;
		}
	}
}

int main(void) {

	bool error = false;
	circuit_t circuit;	//struktura reprezentująca obwód

	std::unordered_set<int32_t> outSignals;

	std::string line;
	int32_t lineNo = 0;
	while (std::getline(std::cin, line)) {
		++lineNo;
		addSignal(circuit, line, lineNo, error);	//error może być zmieniony
	}

	//obwód utworzony (jako zmienna circuit)
	if (error) {
		return 0;
	}

    //obwód poprawny (poza ew. sekwencyjnością)

	std::set<int32_t> inSignals;	//sygnały wejsciowe do obwodu 
	std::vector<int32_t> order;
	bool dag = true;

	topoSort(circuit,order,inSignals,dag);

	if (!dag) {
		std::cout << "Error: sequential logic analysis has not yet been implemented."<<std::endl;
		return 0;
	}

    if (inSignals.size() != 0)
	    runSimulation(circuit,order,inSignals);

	return 0;
}
