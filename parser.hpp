#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <functional>

#include "types.h"

#define nOP 3
#define pInfo 2

using namespace std;

class Parser {
public:

    
    Parser() = default;
    ~Parser() {
        cout << "Parser destructor" << endl;
        delete[] SvectorPerThread;
    }

    bool parse_input_file(const string& filename) {
        ifstream file(filename);
        string line;
        int value; 
        long long value2;

        getline(file, line);
        istringstream iss(line);
        string key;
        iss >> key;

        // Get the node asocciated to each thread  
        if (key != "nodeThread:") {
            cout << "Error: nodeThread not found" << endl;
            file.close();
            return false;
        } 
        while (iss >> value) {
            nodeThread.push_back(value);
        }
        
        getline(file, line);
        iss.clear(); 
        iss.str(line);
        iss >> key;
        
        //Get the operations that are going to be performed in the program 
        //TODO mejorar esto
        if (key != "opType:") {
            cout << "Error: opType not found" << endl;
            file.close();
            return false; 
        }
        NopTot = 0;
        bool flag = 0;
        string op;
        iss >> op;
        for (int i = 0; i < nOP; i++) {
            std::cout << op << std::endl;
            if (op.compare("-")) {
                opType[i] = true;
                NopTot++;
                flag = 1;
            } else {
                opType[i] = false;
            }
            iss >> op;
        }
        if(!flag){
            cout << "No operations to perform" << endl;
            file.close();
            return false;
        }


        getline(file, line);
        iss.clear(); 
        iss.str(line);
        iss >> key;
            
        // Activate/deactivate usage of the private data
        //TODO repensar esto
        if (key == "pData:") {

            iss >> pData;
            if(pData){
                                         
                getline(file, line);
                iss.clear(); 
                iss.str(line);
                iss >> key;
                
                // Get the node of each private vector
                if (key != "nodePVector:") {
                    cout << "Error: nodePVector not found" << endl;
                    file.close();
                    return false;
                } 
                for (int i = 0; i < (int)nodeThread.size(); i++) {
                    iss >> value;
                    nodePVector.push_back(value);
                }
                
                getline(file, line);
                iss.clear(); 
                iss.str(line);
                iss >> key;

                //Get size of each private vector
                if (key != "tamPVector:") {
                    cout << "Error: tamPVector not found" << endl;
                    file.close();
                    return false;
                }
                for (int i = 0; i < (int)nodeThread.size(); i++) {
                    iss >> value2;
                    tamPVector.push_back(value2);
                }
                
            } 
        }

        getline(file, line);
        iss.clear(); 
        iss.str(line);
        iss >> key; 
            
        //Get the node of each shared vector
        if (key != "nodeSVector:") {
            cout << "Error: nodeSVector not found" << endl;
            file.close();
            return false;
        } 
        while (iss >> value) {
            
            nodeSVector.push_back(value);
        }

        getline(file, line);
        iss.clear();
        iss.str(line);
        iss >> key;

        //Get the size of each shared vector
        if (key != "tamSVector:") {
            cout << "Error: tamSVector not found" << endl;
            file.close();
            return false;
        }
        while (iss >> value) {
            tamSVector.push_back(value);
        }

        getline(file, line);
        iss.clear();
        iss.str(line);
        iss >> key;

        //Get the shared vector that each thread is going to process
        if (key != "SvectorPerThread:") {
            cout << "Error: SvectorPerThread not found" << endl;
            file.close();
            return false;
        }

        initialize_SvectorPerThread();
        string group;
        for(int i =0; i < (int)nodeSVector.size(); i++) { 
            getline(iss, group, ',');
            istringstream groupStream(group);
            while (groupStream >> value) { 
                SvectorPerThread[value] = i;
            }
            
        }
        getline(file, line);
        iss.clear();
        iss.str(line);
        iss >> key;

        //Get the number of elements to process by each thread
        if (key != "numSElmProc:") {
            cout << "Error: numSElmProc not found" << endl;
            file.close();
            return false;
        }
        while (iss >> value) {
            numSElmProc.push_back(value);
        }

        file.close();        
        return true;
    }

    //Getter the number of threads
    int get_num_threads() {
        return (int)nodeThread.size();
    }

    //Getter the node of each thread
    int get_node_thread(int thread) {
        return nodeThread[thread];
    }

    //Getter the operations that are going to be performed in the program (1: read, 2: write, 3: read/write)
    bool get_op_type(int op) {
        return opType[op];
    }

    //Getter the number of operations that are going to be performed in the program
    int get_num_op_tot() {
        return NopTot;
    }

    //Getter the usage of the private data
    bool get_p_data() {
        return pData;
    }

    //Getter the node of each private vector
    int get_node_p_vector(int thread) {
        return nodePVector[thread];
    }

    //Getter the size of each private vector
    vectorSize get_tam_p_vector(int thread) {
        return tamPVector[thread];
    }

    //Get number of shared vectors
    int get_num_s_vector() {
        return nodeSVector.size();
    }

    //Getter the node of each shared vector
    int get_node_s_vector(int vector) {
        return nodeSVector[vector];
    }

    //Getter the size of each shared vector
    vectorSize get_tam_s_vector(int vector) {
        return tamSVector[vector];
    }

    //Getter the shared vector that each thread is going to process
    int get_s_vector_per_thread(int thread) {
        return SvectorPerThread[thread];
    }

    //Getter the number of elements to process by each thread
    int get_num_s_elm_proc(int thread) {
        return numSElmProc[thread];
    }


    //Auxiliar function to print the data
    void print() {
        cout << "nodeThread: ";
        for (int value : nodeThread) {
            cout << value << " ";
        }
        cout << endl;

        cout << "opType: ";
        for (int i = 0; i < 3; ++i) {
            cout << opType[i] << " ";
        }
        cout << endl;

        cout << "pData: " << pData << endl;

        cout << "nodePVector: ";
        for (int value : nodePVector) {
            cout << value << " ";
        }
        cout << endl;

        cout << "tamPVector: ";
        for (long long value : tamPVector) {
            cout << value << " ";
        }
        cout << endl;

        cout << "nodeSVector: ";
        for (int value : nodeSVector) {
            cout << value << " ";
        }
        cout << endl;

        cout << "tamSVector: ";
        for (int value : tamSVector) {
            cout << value << " ";
        }
        cout << endl;

        cout << "SvectorPerThread: ";
        for (int i = 0; i < (int)nodeThread.size(); ++i) {
            cout << SvectorPerThread[i] << " ";
        }
        cout << endl;

        cout << "numSElmProc: ";
        for (int value : numSElmProc) {
            cout << value << " ";
        }
        cout << endl << endl;
        }

private:
    vector<int> nodeThread;
    bool opType[nOP];
    int NopTot;
    bool pData;
    vector<int> nodePVector;
    vector<vectorSize> tamPVector;
    vector<int> nodeSVector;
    vector<vectorSize> tamSVector; 
    int* SvectorPerThread;
    vector<vectorSize> numSElmProc;

    void  initialize_SvectorPerThread() {
        int size = (int)nodeThread.size();
        SvectorPerThread = new int[size];

        for (int i = 0; i < size; ++i) {
            SvectorPerThread[i] = -1;
        }
    }
};

#endif // PARSER_HPP