#include "thread.h"
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <string>
#include <tuple>

using namespace std;



/*//////////////////////////////////// Lock codes //////////////////////////////////////

0 = accessing number of living requests
1 = accessing vector of waiting request threads (1 = full)


///////////////////////////////////////////////////////////////////////////////////// */



int requestNum;
//vector<int> requests;

// shared state
int livingRequests;
vector< tuple<int,int> > waiting;





void requester (int numDisk, vector<int> tracks) {
  //for every track
  for(int i = 0; i < tracks.size(); i++){
    
    thread_lock(1);

    //wait till there is a spot in the waiting list
    while(waiting.size() == requestNum){
      thread_wait(1, 0);
    }
    
    //update waiting list
    tuple<int, int> request = (numDisk, tracks[i]);
    waiting.push_back(request);

    //update number of living requests
    thread_lock(0);
    livingRequests++;
    thread_unlock(0);
    
    //wake up servicer
    thread_broadcast(1,1);
    thread_unlock(1);
  }




}

void servicer(void *a) {
  int curSer = -1;
  while(1){
    thread_lock(0);
    if(livingRequests > 0){
      thread_lock(1);
    }
    thread_unlock(0);

    while(waiting.size() != requestNum){
      thread_wait(1, 1);
    }

    // execute request
    // first time
    if(curSer == -1) {
      cout << "service requester " << get<0>(waiting[0]) << " track " << get<1>(waiting[0]) << endl;
      curSer = get<1>(waiting[0]);
      waiting.erase(waiting.begin());
    } else {
      //find nearest to previous track
      int temp = 0;
      for (int i = 1; i < requestNum; i++){
	if(abs(curSer-get<1>(waiting[i])) < abs(curSer-(get<1>(waiting[temp])))){
	  temp = i;
	}
      }
      //remove this track
      cout << "service requester " << get<0>(waiting[temp]) << " track " << get<1>(waiting[temp]) << endl;
      curSer = get<1>(waiting[temp]);
      waiting.erase(waiting.begin() + temp);
      
    }
    
    //check if done, if not, return
    thread_lock(0);
    if((livingRequests == 0) && (waiting.size() == 0)){
      cout << "done" << endl;
      break;
    }
    livingRequests--;
    thread_unlock(0);

    //wake all waiting threads
    thread_broadcast(1, 0);
    thread_unlock(1);
  
  }
}



    /*
// filling vector with requesters
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
    */

int main(int argc, char **argv){
  requestNum = atoi(argv[1]);
  //fillVector(argc, argv);
  //  for (int i = 0; i <= argc
  return 0;
}
