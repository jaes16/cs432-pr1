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
int livingRequests;
vector< tuple<int,int> > waiting;





void requester (void *a) {

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

    thread_lock(1);
    bool exist = false;
    bool contained = true;
    while(contained){
      if((total < requestNum) && (waiting.size() < requestNum)){
	break;
      }
      for(int i = 0; i < waiting.size(); i++){
	if(get<0>(waiting[i]) == numDisk){
	  exist = true;
	}
      }
      if(exist){
	contained = true;	
	/*	cout<< "//////////////////////" << endl;
	for(int j = 0; j < waiting.size(); j++){
	  cout << "disk" << get<0>(waiting[j]) << "track " << get<1>(waiting[j]) << endl;
	}
	cout<< "//////////////////////" << endl;
	cout << "here: " << numDisk << endl;
	*/
	thread_wait(1, 0);
      }else {
	contained = false;
      }
      exist = false;
    }
   
    //cout << "req" << c << ": " << s << endl;

    //wait till there is a spot in the waiting list
    while(waiting.size() == requestNum){
      if(total < requestNum){
	break;
      }
      cout << "here: " << numDisk << endl;
      thread_wait(1, 0);
    }

    //update waiting list
    tuple<int, int> request = make_tuple(numDisk, track);
    waiting.push_back(request);
    //cout << "waiting size: " << waiting.size() << endl;
    cout << "requester "<< numDisk << " track " << track << endl;


    //wake up servicer
    thread_broadcast(1,1);
    thread_broadcast(1,0);
    thread_unlock(1);
  
  }
  


}
/*
void servicer(void *a) {
  int curSer = -1;
  while(1){

    thread_lock(0);
    if(livingRequests > 0){
      thread_lock(1);
    }
    thread_unlock(0);

    while(waiting.size() < requestNum){
      thread_wait(1, 1);
    }

    // execute request
    // first time
    if(curSer == -1) {
      cout << "service requester " << get<0>(waiting[0]) << " track " << get<1>(waiting[0]) << endl;
      curSer = get<1>(waiting[0]);
      waiting.erase(waiting.begin() + 0);
    } else {
      //find nearest to previous track
      int temp = 0;
      for (int i = 1; i < requestNum; i++){
	if(abs(curSer-(get<1>(waiting[i]))) < abs(curSer-(get<1>(waiting[temp])))){
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



void servicer(void *a) {
  int curSer = -1;
  while(requesters > 0){
    thread_lock(0);
    if(livingRequests > 0){
      thread_lock(1);
    }
    thread_unlock(0);

    //cout << "requesters: " << requesters << endl;
    //cout << "waiting.size() = " << waiting.size() << endl;
    while(waiting.size() < requestNum){
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
*/





void servicer(void *a) {
  int curSer = -1;
  while(total >=  requestNum){
    thread_lock(1);
    cout << "///////////" << total << ", " << requestNum << endl;
    while (waiting.size() < requestNum){
      if(total == requestNum){
	break;
      }


      thread_wait(1,1);
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


          cout<< "//////////////////////" << endl;
      for(int j = 0; j < waiting.size(); j++){
	cout << "disk" << get<0>(waiting[j]) << "track " << get<1>(waiting[j]) << endl;
      }
      cout<< "//////////////////////" << endl;


    //wake all waiting threads
    thread_broadcast(1, 0);
    thread_unlock(1);
    total--;

  }
  
  cout << "hi///////////////////////////" << endl;

  for(int i = total; i > 0; i--){

     cout<< "//////////////////////" << endl;
      for(int j = 0; j <= waiting.size(); j++){
	cout << "disk" << get<0>(waiting[j]) << "track " << get<1>(waiting[j]) << endl;
      }
      cout<< "//////////////////////" << endl;      


    if(i == 1){
    cout << "service requester " << get<0>(waiting[0]) << " track " << get<1>(waiting[0]) << endl;
    break;
    }

    
    thread_lock(1);
    /*    while (waiting.size() < i){
      cout<< "here: servicer" << endl;
      
      cout<< "//////////////////////" << endl;
      for(int j = 0; j < waiting.size(); j++){
	cout << "disk" << get<0>(waiting[j]) << "track " << get<1>(waiting[j]) << endl;
      }
      cout<< "//////////////////////" << endl;      
      
      thread_wait(1,1);
    }
  */
    // execute request
    //find nearest to previous track
    int temp = 0;
    for (int j = 1; j < i; j++){
      if(abs(curSer-get<1>(waiting[j])) < abs(curSer-(get<1>(waiting[temp])))){
	temp = j;
      }
    }
    //remove this track
    cout << "service requester " << get<0>(waiting[temp]) << " track " << get<1>(waiting[temp]) << endl;
    curSer = get<1>(waiting[temp]);
    waiting.erase(waiting.begin() + temp);
    //cout << "waiting size servicer: " << waiting.size() << endl;



    //wake all waiting threads
    thread_broadcast(1, 0);
    thread_unlock(1);

    /*    cout<< "//////////////////////" << endl;
    for(int j = 0; j < waiting.size(); j++){
      cout << get<0>(waiting[j]) << endl;
    }
    cout<< "//////////////////////" << endl; */
  }

  thread_broadcast(5,1);

}




void initializer(void *a) {
  int argc = (intptr_t)a;
  int maxRequesters = 0;
  for(int i = 0; i < argc; i++){ //****************made this maxRequesters thing
    maxRequesters++;
  }
  ifstream f;
  f.open("disk.in0");
  int count = 0;
  string s;
  while(f>>s){
    count++;
  }
  total = count * maxRequesters;

  
  for(int i = 0; i < argc; i++){
    thread_create((thread_startfunc_t) requester, (void *)(intptr_t)i);
  }
  thread_create((thread_startfunc_t) servicer, (void *) (intptr_t)(count * maxRequesters));
}



int main(int argc, char **argv){
  requestNum = atoi(argv[1]);
  thread_libinit((thread_startfunc_t) initializer, (void *)(intptr_t)(argc - 2));

  //fillVector(argc, argv);
  //  for (int i = 0; i <= argc
  return 0;
}
