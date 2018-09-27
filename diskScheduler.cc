#include "thread.h"
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <string>

using namespace std;

int* requester;
vector<int> requests;

void fillVector(int argc, char **argv){
  if(argc > 1){
    queue<string> fileNames;
    for(int i = 2; i < argc; i++){
      fileNames.push(argv[i]);
    }
    int line = 0;
    int currentFile = 0;
    bool pushing;
    while(!fileNames.empty()){
      pushing = true;
      ifstream fn;
      string fName = fileNames.front();
      fileNames.pop();
      fn.open(fName);
      if(fn.is_open()){
	int num;
	for(int count = 0; count <= line; count++){
	  if(!(fn >> num)){
	    pushing = false;
	    break;
	  }
	}
	if(pushing){
	  requests.push_back(num);
	  fileNames.push(fName);
	}
	currentFile++;
	if(currentFile == argc - 2){
	  currentFile = 0;
	  line++;
	}
      }
    }
  }
}


int main(int argc, char **argv){
  requester = new int[atoi(argv[1])];
  fillVector(argc, argv);
  for(int i = 0; i < requests.size(); i++){
    cout << requests[i] << endl;
  }
  return 0;
}
