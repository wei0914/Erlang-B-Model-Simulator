#include<iostream>
#include <vector>
#include<random>
#include "omp.h"
#include<iomanip>
#include<numeric>
#include <sstream> 
#include <fstream>

using namespace std;
int main() {
    int item = 0;
    //random
    random_device rd;  //Will be used to obtain a seed for the random number engine
    mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    uniform_real_distribution<> dis(0.0, 1.0);
    unsigned int block = 0;
    unsigned int arrival = 0;
    int impos  =0;
    double pdfa;    //pdf_arrival
    double pdfs;    //pdf_service
    double proa;    //arrival prob.
    double pros;
    double bp;
    double ratio = 0;
    double sum;
    double mean;
    
    vector<double> bpt[3];//二維陣列包含3個向量
    vector<double> rt[3];//二維陣列包含3個向量
    vector<double> bpt2[3];
    
    vector<int> s = { 1, 5, 10 }; //# of server
    vector<double> ten;
    unsigned int server =0;
    unsigned int queue = 0;
    unsigned int space = 0;
    
    for (auto i : s) { //server
        for (double j = 0.01; j <= 10; j *= 10) {//lumda 4 times
            for (double k = 0.01; k <= 10.24; k *= 4) {//u 6 times
                //time unit(ut)
                pdfa = 1 - exp(-j);
                pdfs = 1 - exp(-k);
                for (int m = 0; m < 10; m++) {
                    for (unsigned int t = 1; t <= 100000; t++) {
                        proa = dis(gen);
                        //cout << proa << endl;
                        if (server > 0) {//someone in server
                            for (int num = server; num > 0; num--) {
                                pros = dis(gen);
                                //cout << "i:" << t << ", " << pros << endl;
                                if (pros <= pdfs) { //some leave
                                    server--;
                                }
                            }
                        }
                        if (proa <= pdfa) {//can enter
                            if (server == i) { //server full
                                block++;
                            }
                            else {
                                server++;
                                arrival++;
                            }
                        }
                        else {
                            ;
                        }
                        //cout << "i:" << t << ", server: " << server << endl;
                    }
                    bp = (double)block / (double)arrival;
                    ten.push_back(bp);
                }
                sum = std::accumulate(std::begin(ten), std::end(ten), 0.0);
                mean = sum / ten.size(); //均值      
                ratio = (double)j / (double)k;           

                std::cout <<i<<"  "<<j<<" "<< k <<endl;               
                //std::cout << "bp:" << mean << endl;
                //std::cout << "r:" << ratio << endl;
                //std::cout << endl;
                            
                if (i == 1) {
                    bpt[0].push_back(mean);
                    rt[0].push_back(ratio);
                }
                else if (i == 5) {
                    bpt[1].push_back(mean);
                    rt[1].push_back(ratio);
                }
                else if (i == 10) {
                    bpt[2].push_back(mean);
                    rt[2].push_back(ratio);
                }
                //cout << "finish 1" << endl;
                sum = 0;
                mean = 0;
                ten.clear();
                block = 0;
                arrival = 0;
            }
        }
    }
    

    
//queue = s***********************
#pragma omp parallel for
    for (auto i : s) { //server
        for (double j = 0.01; j <= 10; j *= 10) {//lumda 4 times
            for (double k = 0.01; k <= 10.24; k *= 4) {//u 6 times
                //time unit(ut)
                pdfa = 1 - exp(-j);
                pdfs = 1 - exp(-k);
                for (int m = 0; m < 10; m++) {
                    for (unsigned int t = 1; t <= 100000; t++) {
                        //cout << t << endl;
                        proa = dis(gen);
                        if (server > 0) {//someone in server
                            for (int num = server; num > 0; num--) {
                                pros = dis(gen);
                                if (pros <= pdfs) { //some leave
                                    server--;
                                    //cout << "server--" << endl;
                                }
                            }
                        }
                        //queue遞補
                        space = i - server;
                        if (space > 0) {//server not empty
                            if (space <= queue) {//queue 部分遞補
                                server = i;
                                queue -= space;
                            }
                            else { //queue < space queue 全部遞補
                                server = server + queue;
                                queue = 0;
                            }
                        }
                        //arrival
                        if (proa <= pdfa) {//can enter
                            if (server == i && queue == i) { //server, queue is full
                                block++;
                            }
                            else if (server < i && queue == 0) {//server not full, queue = 0, 進server
                                server++;
                                arrival++;
                            }
                            else if (queue < i && server == i) {//server 滿, queue有位子, 進queue
                                queue++;
                                arrival++;
                            }
                        }
                        else {
                            impos++;
                        }
                        //cout << "queue: " << queue << ", server: " << server << endl;
                    }
                    bp = (double)block / (double)arrival;
                    ten.push_back(bp);
                }
                sum = std::accumulate(std::begin(ten), std::end(ten), 0.0);
                mean = sum / ten.size(); //均值  
                ratio = (double)j / (double)k;
                
                std::cout << i << "  " << j << " " << k << endl;                
                //std::cout << "bp:" << mean << endl;
                //std::cout << "r:" << ratio << endl;
                if (i == 1) {
                    bpt2[0].push_back(mean);                 
                }
                else if (i == 5) {
                    bpt2[1].push_back(mean);                    
                }
                else if (i == 10) {
                    bpt2[2].push_back(mean);                  
                }
                block = 0;
                arrival = 0;
                sum = 0;
                mean = 0;
                ten.clear();
            }
        }
    }

    //output
    vector<double>  ::iterator iter = bpt[0].begin();
    std::ofstream outFile;
    outFile.open("./bptable.csv", std::ios::out);
    outFile << "Queue = 0" << std::endl;
    outFile << "server" << "," << "blocking probability & Erlang" << std::endl;
    //s = 1
    outFile << "s = 1" << ',';
    for (int ix = 0; iter != bpt[0].end(); ++iter, ++ix) {
        outFile << *iter << ',';
    }
    outFile << std::endl << ",";
    for (iter = rt[0].begin(); iter != rt[0].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << std::endl;

    //s = 5
    outFile << "s = 5" << ',';
    for (iter = bpt[1].begin(); iter != bpt[1].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << ",";
    for (iter = rt[1].begin(); iter != rt[1].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << std::endl;

    //s = 10
    outFile << "s = 10" << ',';
    for (iter = bpt[2].begin(); iter != bpt[2].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << ",";
    for (iter = rt[2].begin(); iter != rt[2].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << std::endl;

    //queue = s
    outFile << "Queue = S" << std::endl;
    outFile << "server" << "," << "blocking probability & Erlang" << std::endl;
    //s = 1
    outFile << "s = 1" << ',';
    for (iter = bpt2[0].begin(); iter != bpt2[0].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << ",";
    for (iter = rt[0].begin(); iter != rt[0].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << std::endl;

    //s = 5
    outFile << "s = 5" << ',';
    for (iter = bpt2[1].begin(); iter != bpt2[1].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << ",";
    for (iter = rt[1].begin(); iter != rt[1].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << std::endl;

    //s = 10
    outFile << "s = 10" << ',';
    for (iter = bpt2[2].begin(); iter != bpt2[2].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << ",";
    for (iter = rt[2].begin(); iter != rt[2].end(); ++iter) {
        outFile << *iter << ',';
    }
    outFile << std::endl << std::endl;

    outFile.close();

//for testing
/*
    int i = 1;      //server size
    double j = 0.1;   //lumbda
    double k = 2.56;  //u

    pdfa = 1 - exp(-j);
    pdfs = 1 - exp(-k);
    for (unsigned int t = 1; t <= 100000; t++) {
        proa = dis(gen);
        //cout << proa << endl;
        if (server > 0) {//someone in server
            for (int num = server; num > 0; num--) {
                pros = dis(gen);
                //cout << "i:" << t << ", " << pros << endl;
                if (pros <= pdfs) { //some leave
                    server--;
                }
            }
        }
        if (proa <= pdfa) {//can enter
            if (server == i) { //server full
                block++;
            }
            else {
                server++;
                arrival++;
            }
        }
        else {
            ;
            //impos++;
        }
        cout << "i:" << t << ", server: " << server << endl;
    }
    
    bp = (double)block / (double)arrival;
    std::cout << "impos:" << impos << endl;
    std::cout << "arrival:" << arrival << endl;
    std::cout << "block:" << block << endl;
    std::cout << "bp:" << bp << endl;
    return 0;
  */  
}

