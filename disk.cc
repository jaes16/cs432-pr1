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


int total;
int requestNum;
//vector<int> requests;

// shared state
int livingRequests = 0;
vector< tuple<int,int> > waiting;
bool done = false;




void requester (void *a) {

  // read in file
  int numDisk = (intptr_t) a;
  char c = 48 + numDisk;
  string fn = "disk.in";
  fn += c;
  ifstream file;
  file.open(fn);
  int track;
  char s[2048];

  thread_lock(1000);
  // for every track
  while(file >> s){
    track = atoi(s);

    thread_lock(1000);

    //wait till there is a spot in the waiting list
    while(waiting.size() == requestNum){
      //      cout << "requester" << numDisk << " waiting" << endl;
      thread_wait(1000, 0);
    }

    
    //update waiting list
    tuple<int, int> request = make_tuple(numDisk, track);
    waiting.push_back(request);
    //cout << "waiting size: " << waiting.size() << endl;
    cout << "requester "<< numDisk << " track " << track << endl;


    //wake up servicer and other requesters
    thread_signal(1000, 1);
    thread_wait(1000, (numDisk + 100));


  }
  

  livingRequests--;
  thread_signal(1000, 1);

  thread_unlock(1000);  
}

void servicer(void *a) {
  int curSer = -1;
  int signalTarget = -1;

  thread_lock(1000);
  while(livingRequests > 0){


    //test
    //    cout << livingRequests << endl;
    //cout << waiting.size() << endl;


    
    while (waiting.size() < min(livingRequests, requestNum)){
      //      cout << "servicer waiting at " << waiting.size() << endl;

      thread_wait(1000,1);

    }

    if(livingRequests == 0){
      break;
    }
    

    
    // execute request
    // first time
    if(curSer == -1) {
      signalTarget = 0;
      cout << "service requester " << get<0>(waiting[0]) << " track " << get<1>(waiting[0]) << endl;
      curSer = get<1>(waiting[0]);
      waiting.erase(waiting.begin());
    }
    else {
      //find nearest to previous track
      int temp = 0;
      for (int i = 0; i < waiting.size(); i++){
	if(abs(curSer-get<1>(waiting[i])) < abs(curSer-(get<1>(waiting[temp])))){
	  temp = i;
	}
      }
      signalTarget = get<0>(waiting[temp]);
      //remove this track
      cout << "service requester " << get<0>(waiting[temp]) << " track " << get<1>(waiting[temp]) << endl;
      curSer = get<1>(waiting[temp]);
      waiting.erase(waiting.begin() + temp);
      //cout << "waiting size servicer: " << waiting.size() << endl;
    }
      
      
    //wake all waiting threads
    
    thread_signal(1000, (signalTarget+100));
    thread_signal(1000, 0);
    
  }


    thread_unlock(1000);  
      
}





void initializer(void *a) {
  int argc = (intptr_t)a;
  for(int i = 0; i < argc; i++){ //****************made this maxRequesters thing
    livingRequests++;
  }  
  for(int i = 0; i < argc; i++){
    thread_create((thread_startfunc_t) requester, (void *)(intptr_t)i);
  }
  thread_create((thread_startfunc_t) servicer, (void *) (intptr_t)a);
}



int main(int argc, char **argv){
  requestNum = atoi(argv[1]);
  thread_libinit((thread_startfunc_t) initializer, (void *)(intptr_t)(argc - 2));

  //fillVector(argc, argv);
  //  for (int i = 0; i <= argc
  return 0;
}
