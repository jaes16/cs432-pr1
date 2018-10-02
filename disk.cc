#include "thread.h"
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <string>
#include <tuple>
#include <algorithm>

using namespace std;



/*//////////////////////////////////// Lock codes //////////////////////////////////////

0 = accessing number of living requests
1 = accessing vector of waiting request threads (1 = full)
2 = accessing previous requesters ***************
3 = accessing living requesters   ***************


///////////////////////////////////////////////////////////////////////////////////// */



int requestNum;
int maxRequesters = 0;  //************** dis new
//vector<int> requests;

// shared state
int livingRequests;
int requesters = 0;  //****************
vector< tuple<int,int> > waiting;
vector<string> previousRequesters; //**************** made this to keep track of who is next to read


void requester (void *a) {
  thread_lock(3);    //*************changed this stuff
  requesters++;
  thread_unlock(3);  //*************
  //for every track
  int numDisk = (intptr_t) a;
  char c = 48 + numDisk;
  //cout << numDisk << endl;
  string fn = "disk.in";
  fn += c;
  
  ifstream file;
  file.open(fn);
  int track;
  char s[2048];
  while(file >> s){
    track = atoi(s);

    thread_lock(2); //check to see if previous requester was this requester **************all this down
    bool contains = false;
    for(int i = 0; i < previousRequesters.size(); i++){
      while(previousRequesters[i].compare(fn) == 0){
	contains = true;
	thread_wait(2,3);
      }
    }
    if(!contains){
      previousRequesters.push_back(fn);
    } // *********************************to here up

    //cout << "req" << c << ": " << s << endl;
  
    thread_lock(1);

    //wait till there is a spot in the waiting list
    while(waiting.size() == requestNum){
      thread_wait(1, 0);
    }
    
    //update waiting list
    tuple<int, int> request = make_tuple(numDisk, track);
    waiting.push_back(request);
    //cout << "waiting size: " << waiting.size() << endl;
    cout << "requester "<< get<0>(request) << " track " << s << endl;
    //update number of living requests
    thread_lock(0);
    livingRequests++;
    thread_unlock(0);
    
    //wake up servicer
    thread_broadcast(1,1);
    thread_unlock(1);

    //wake up other requesters ********************* this is new
    if(previousRequesters.size() == maxRequesters){
      previousRequesters.erase(previousRequesters.begin(), previousRequesters.begin()+requestNum);
    }
    thread_broadcast(2,3);
    thread_unlock(2);
  }

  thread_lock(3);
  requesters--;
  thread_unlock(3);


}

void servicer(void *a) {
  int curSer = -1;
  while(requesters > 0){ //***************changed this
    thread_lock(0);
    if(livingRequests > 0){
      thread_lock(1);
    }
    thread_unlock(0);
    
    //cout << "requesters: " << requesters << endl;
    //cout << "waiting.size() = " << waiting.size() << endl;
    while(waiting.size() < min(requestNum, maxRequesters)){ //*****************changed logic in here
      thread_wait(1, 1);
      thread_lock(3);
      if(requesters < maxRequesters){
	maxRequesters = requesters;
      }
      thread_unlock(3);
      if(requesters == 0){
	return;
      }
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
      //cout << "waiting size servicer: " << waiting.size() << endl;
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

void initializer(void *a) {
  int argc = (intptr_t)a;
  for(int i = 0; i < argc; i++){ //****************made this maxRequesters thing
    maxRequesters++;
  }
  for(int i = 0; i < argc; i++){
    thread_create((thread_startfunc_t) requester, (void *)(intptr_t)i); 
  }
  thread_create((thread_startfunc_t) servicer, (void *) a); 
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
  thread_libinit((thread_startfunc_t) initializer, (void *)(intptr_t)(argc - 2));

  //fillVector(argc, argv);
  //  for (int i = 0; i <= argc
  return 0;
}
