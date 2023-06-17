#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <unordered_map>
#include "types.h"
#define pInfo 2

using namespace std;

class Parser {
public:

    Parser() = default;
    ~Parser() {
        delete[] vectorPerThread;
    }
    void parserError(string error) {
        cout << "Parser error: " << error << endl;
        exit(1);
    }

    //Function to parse the input file
    bool parse_input_file(const string& filename) {
        ifstream file(filename);
        string line;
        string stringValue;
        vectorSize intValue; 

        // Get the node asocciated to each thread  
        getline(file, line);
        istringstream iss(line);    
        iss >> stringValue;        
        if (stringValue != "opType:") {
            parserError("OpType not found");
            file.close();
            return false; 
        }
        string op;
        while (iss >> op) {
            opType.push_back(op);
        }
        if(!opType.size()){
            parserError("No operations to perform");
            file.close();
            return false;
        }

        //Get the operations to be performed in the program 
        getline(file, line);
        iss.clear(); 
        iss.str(line);
        iss >> stringValue;
        if (stringValue != "nodePerThread:") {
            parserError("nodePerThread not found");
            file.close();
            return false;
        } 
        while (iss >> intValue) {
            nodePerThread.push_back(intValue);
        }
        
        //Get the node of each vector
        getline(file, line);
        iss.clear(); 
        iss.str(line);
        iss >> stringValue;
        if (stringValue != "nodePerVector:") {
            parserError("nodePerVector not found");
            file.close();
            return false;
        } 
        while (iss >> intValue) {
            nodePerVector.push_back(intValue);
        }

        //Get the size of each vector
        getline(file, line);
        iss.clear();
        iss.str(line);
        iss >> stringValue;
        if (stringValue != "tamPerVector:") {
            parserError("tamPerVector not found");
            file.close();
            return false;
        }
        while (iss >> intValue) {
            tamPerVector.push_back(intValue);
        }

        //Get vector that each thread is going to process
        getline(file, line);
        iss.clear();
        iss.str(line);
        iss >> stringValue;        
        if (stringValue != "vectorPerThread:") {
            parserError("vectorPerThread not found");
            file.close();
            return false;
        }
        initialize_vectorPerThread();
        string group;
        for(int i =0; i < (int)nodePerVector.size(); i++) { 
            getline(iss, group, ',');
            istringstream groupStream(group);
            while (groupStream >> intValue) { 
                vectorPerThread[intValue] = i;
            }
        }

        //Get the type of summary to be performed
        getline(file, line);
        iss.clear();
        iss.str(line);
        iss >> stringValue;
        if (stringValue != "summaryType:") {
            parserError("summaryType not found");
            file.close();
            return false;
        }
        iss >> summaryType;       

        //Get the speedups to be calculated
        getline(file, line);
        iss.clear();
        iss.str(line);
        iss >> stringValue;
        if (stringValue != "speedupCalc:") {
            file.close();
            return false;
        }
        while (iss >> stringValue) {
            auto pos = stringValue.find("/");
            //Check if the operation is in the list of operations
            string aux = stringValue.substr(0, pos);
            if(opType.end() == find(opType.begin(), opType.end(), aux)){
                parserError("Operation \"" + aux + "\" not set for execution");
                file.close();
                return false;
            }
            aux = stringValue.substr(pos+1, stringValue.size());
            if(opType.end() == find(opType.begin(), opType.end(), aux)){
                parserError("Operation \"" + aux + "\" not set for execution");
                file.close();
                return false;
            }
            speedupCalcList.push_back(stringValue);
        }

        //Get number of times to repeat the execution
        getline(file, line);
        iss.clear();
        iss.str(line);
        iss >> stringValue;
        if (stringValue != "numIter:") {
            parserError("numIter not found");
            file.close();
            return false;
        }
        iss >> intValue;
        numIter = intValue;

        

        file.close();        
        return true;
        
        
    }

    //Getter the number of threads
    int get_num_threads() {
        return (int)nodePerThread.size();
    }

    //Getter the node of each thread
    int get_node_thread(int thread) {
        return nodePerThread[thread];
    }

    //Getter the operations that are going to be performed in the program (1: read, 2: write, 3: read/write)
    vector<string> get_op_list() {
        return opType;
    }

    //Get number of shared vectors
    int get_num_s_vector() {
        return nodePerVector.size();
    }

    //Getter the node of each shared vector
    int get_node_s_vector(int vector) {
        return nodePerVector[vector];
    }

    //Getter the size of each shared vector
    vectorSize get_tam_s_vector(int vector) {
        return tamPerVector[vector];
    }

    //Getter the shared vector that each thread is going to process
    int get_s_vector_per_thread(int thread) {
        return vectorPerThread[thread];
    }

    //Get the number of times to repeat the execution
    int get_num_iter() {
        return numIter;
    }

    //Get the type of summary to be performed
    string get_summary_type() {
        return summaryType;
    }

    //Get the speedups to be calculated
    vector<string> get_speedup_calc() {
        return speedupCalcList;
    }

    //Print parser data
    void print(ofstream& outfile) {
        
        outfile << setw(18) << " " << "--Execution data--" << endl;
        outfile << "+ Vectors data" << endl;
        for (int i = 0; i < (int)nodePerVector.size(); ++i) {
            outfile << " - Vector "  << setw(6) << left << i;
            outfile << "node: " << setw(6) << left << nodePerVector[i];
            outfile << "size: " << tamPerVector[i] << endl;
        }
        outfile << "\n\n";

        outfile << "+ Threads data" << endl;
        for (int i = 0; i < (int)nodePerThread.size(); ++i) {
            outfile << " - Thread " << setw(6) << left << i;
            outfile << "node: " << setw(6) << left << nodePerThread[i];
            outfile << "vector: " << vectorPerThread[i];
            outfile << endl;
        }
        outfile << "\n";
        outfile << "+ Operation data" << endl;
        outfile << " - Operations to be performed: ";
        for (int i = 0; i < (int)opType.size(); ++i) {
            outfile << opType[i] << ", ";
        }
        outfile << endl;
        outfile << " - type of summary: " << summaryType << endl;
        outfile << "\n";
    }

    //Print parser data to be used in scripting
    void printP(){
        int i;
        cout << "Execution data:" << endl;
        cout << "Vectors:";
        for (i = 0; i < (int)nodePerVector.size()-1; ++i) {
            cout << "[" << nodePerVector[i] << "," << tamPerVector[i] << "],";
        }
        cout << "[" << nodePerVector[nodePerVector.size()-1] << "," << tamPerVector[nodePerVector.size()-1] << "]";
        cout << endl;
        cout << "Threads:";
        for (i = 0; i < (int)nodePerThread.size()-1; ++i) {
            cout << "[" << nodePerThread[i] << "," << vectorPerThread[i] << "],";
        }
        cout << "[" << nodePerThread[nodePerThread.size()-1] << "," << vectorPerThread[nodePerThread.size()-1] << "]";
        cout << endl;
        cout << "Operations:";
        
        for (i = 0; i < (int)opType.size()-1; ++i) {
            cout << opType[i] << ",";
        }
        cout << opType[opType.size()-1];
        cout << endl;

        cout << "Summary:" << summaryType << endl;
        cout << "SpeedupCalc:"; 
        for (i = 0; i < (int)speedupCalcList.size()-1; ++i) {
            cout << speedupCalcList[i] << ",";
        }
        cout << speedupCalcList[speedupCalcList.size()-1] << endl;

    }

private:
    vector<int> nodePerThread;
    vector<string> opType;
    vector<int> nodePerVector;
    vector<vectorSize> tamPerVector; 
    int* vectorPerThread;
    string summaryType;
    vector <string> speedupCalcList;
    int numIter;
    void  initialize_vectorPerThread() {
        int size = (int)nodePerThread.size();
        vectorPerThread = new int[size];
        for (int i = 0; i < size; ++i) {
            vectorPerThread[i] = -1;
        }
    }
};
#endif // PARSER_HPP