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

  // read in file
  int numDisk = (intptr_t) a;
  char c = 48 + numDisk;
  string fn = "disk.in";
  fn += c;
  ifstream file;
  file.open(fn);
  int track;
  char s[2048];

  // for every track
  while(file >> s){
    track = atoi(s);

    //test
    //    cout << "here: " << numDisk << ", " << s << endl;


    thread_lock(1);
    bool exist = false;
    bool contained = true;

    // while the waiting list has another track from this requester, wait
    while(contained){

      //test
      //      cout << total << ", " <<  requestNum << ", " << waiting.size() << endl;

      // however, if there are less total tracks than the size of the swaiting list, put in anyway
      thread_lock(2);
      if((total <= requestNum) && (waiting.size() < requestNum)){
	thread_unlock(2);
	break;
      }
      thread_unlock(2);

      //checking if waiting list has another track from this requester
      for(int i = 0; i < waiting.size(); i++){
	if(get<0>(waiting[i]) == numDisk){
	  exist = true;
	}
      }

      //waiting list has another track from this requester: wait till servicer services a track, and check again.
      if(exist){
	contained = true;	

	thread_wait(1, 0);

	// else exist
      }else {
	contained = false;
      }
      // for next check.
      exist = false;
    }
   
    //cout << "req" << c << ": " << s << endl;

    
    //wait till there is a spot in the waiting list
    while(waiting.size() == requestNum){
      thread_lock(2);
      if(total < requestNum){
	thread_unlock(2);
	break;
      }
      thread_unlock(2);
      //      cout << "here: " << numDisk << endl;
      thread_wait(1, 0);
    }

    
    //update waiting list
    tuple<int, int> request = make_tuple(numDisk, track);
    waiting.push_back(request);
    //cout << "waiting size: " << waiting.size() << endl;
    cout << "requester "<< numDisk << " track " << track << endl;


    //wake up servicer and other requesters
    thread_broadcast(1,1);
    thread_broadcast(1,0);
    thread_unlock(1);
  
  }
  


}

void servicer(void *a) {
  int curSer = -1;

  // as long as total number of requests can fill the waiting list
  thread_lock(2);
  while(total >=  requestNum){
    thread_unlock(2);
    thread_lock(1);
    //    cout << "///////////" << total << ", " << requestNum << endl;

    // while waiting list is not full, wait
    while (waiting.size() < requestNum){
      //      cout << "servicer waiting at " << waiting.size() << endl;
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


    /*    cout<< "//////////////////////" << endl;
      for(int j = 0; j < waiting.size(); j++){
	cout << "disk" << get<0>(waiting[j]) << "track " << get<1>(waiting[j]) << endl;
      }
      cout<< "//////////////////////" << endl;
    */

    //wake all waiting threads
    thread_broadcast(1, 0);
    thread_unlock(1);
    total--;

  }
  thread_unlock(2);
  
  //  cout << "hi///////////////////////////" << endl;

  for(int i = requestNum-2; i >= 0; i--){

    if(i == 0){
    cout << "service requester " << get<0>(waiting[0]) << " track " << get<1>(waiting[0]) << endl;
    break;
    }

    
    thread_lock(1);

    // execute request
    //find nearest to previous track
    int temp = 0;
    for (int j = 0; j < waiting.size(); j++){
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
