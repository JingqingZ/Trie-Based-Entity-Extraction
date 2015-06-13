#include "AEE.h"
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
    AEE aee;

    vector<EDExtractResult> resultED;
    vector<JaccardExtractResult> resultJaccard;

    unsigned edThreshold = 2;
    //double jaccardThreshold = 0.85;

    aee.createIndex(argv[1]);
    //aee.aeeJaccard(argv[2], jaccardThreshold, resultJaccard);
    char* test = "baiduxbaiduxbaiduj";
    cout << test << endl;
    aee.aeeED(test, edThreshold, resultED);
    
    for (int i = 0 ; i < resultED.size(); i++) {
    	cout << resultED[i].id << " " 
    	     << resultED[i].pos << " " 
    	     << resultED[i].len << " " 
    	     << resultED[i].sim << endl; 
    }
    
    cout << "------------" << endl;
    aee.aeeED(test, edThreshold, resultED);
    for (int i = 0 ; i < resultED.size(); i++) {
    	cout << resultED[i].id << " " 
    	     << resultED[i].pos << " " 
    	     << resultED[i].len << " " 
    	     << resultED[i].sim << endl; 
    }
    
    return 0;
}
