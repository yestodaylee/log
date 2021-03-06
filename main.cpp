/*
 * Copyright [2012-2015] DaSE@ECNU
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * /log/main.cpp
 *
 *  Created on: 2016年2月24日
 *      Author: lizhif
 *		   Email: 885626704@qq.com
 * 
 * Description:
 *
 */
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <map>
#include <atomic>
#include <memory>
#include <time.h>
#include <sys/time.h>
#include "unistd.h"
#include "sys/time.h"
#include "log_manager.hpp"
#include "lock_free.hpp"
#include "gc_manager.hpp"
#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "tran_manager.hpp"
using std::cin;
using std::cout;
using std::endl;
using std::thread;
using std::vector;
using std::map;
using std::shared_ptr;
using std::make_shared;
using std::atomic_is_lock_free;
 /*
void task1(int id) {
  for(auto p=0;p<10000;p++ ) {
    logm.LogBegin(id*10000 + p);
    logm.LogCommit(id*10000 + p);
  }
}
*/
/*
int main (){
  logm.Startup();
  sleep(1);
  vector<thread> v;
  for(auto i= 0;i<100;i++)
    v.push_back(thread(task1,i));
  for(auto i = 0;i<100;i++)
      v[i].join();
}
*/
const int size = 3000;
char buffer[size];

void task2(int id) {
  for(auto i=0;i<100 ;i++) {
    LogService::LogAppend("ssss\n");
    //LogManager:: LogData(id,1,2,3,buffer,size);
    //LogManager::FlushToDisk();
    //logm.LogCommit(id);
    //logm.LogAbort(id);
  }
}
/*
int main() {
  for(auto i=0;i<size-1;i++)
    buffer[i]='a';
  buffer[size-1]=0;
  LogManager::Startup();

  vector<thread> v;
  for(auto i=0;i<100;i++)
    v.push_back(thread(task2,i));
  for(auto i=0;i<100;i++)
    v[i].join();
}
*/
/*
int main(){

  vector<int> v1 = {1,2,3};
  vector<int> v2 = {1};
  Trans t;
  cout << sizeof(v1) << endl;
  cout << sizeof(v2) << endl;
  cout << sizeof(&v1) << endl;
  cout << sizeof(t) << endl;
  cout << sizeof(Strip) << endl;
  cout << trans.NPart << endl;
  TransService::Startup();

}*/
/*
map<UInt64,atomic<int>> kv;
void task3(int id) {
  for (auto i = 0;i<100; i++)
     kv[id] = i;
}
int main() {
  for(UInt64 i = 0; i < 100; i++)
    kv[i] = 0;
  cout << "1" << endl;
  vector<thread> tasks;
  for (auto i = 0;i<100;i++)
    tasks.push_back(thread(task3,i/10));
  for (auto i = 0;i<100;i++)
    tasks[i].join();
}

*/

template<typename T>
class SmartPtr {
 public:
  T* row_ptr;
  int* use;
  SmartPtr(T* ptr):row_ptr(ptr),use(new int(1)) { }
  SmartPtr(const SmartPtr<T> & ptr):row_ptr(ptr.row_ptr){
    use=ptr.use;
    __sync_add_and_fetch(use,1);
   // ++*use;
  }
  SmartPtr & operator = (const SmartPtr & rhs) {
    //++*rhs.use;
    __sync_add_and_fetch(use,1);
    if (/*--*use == 0*/__sync_sub_and_fetch(use,1) == 0) {
      delete row_ptr;
      delete use;
    }
    row_ptr = rhs.row_ptr;
    use = rhs.use;
    return *this;
  }
  ~SmartPtr(){
    if(/*--*use == 0*/__sync_sub_and_fetch(use,1) == 0) {
      delete row_ptr;
      delete use;
    }
  }
};

SmartPtr<int> origin1(new int(5));
shared_ptr<int> origin2(new int(5));
void task4(int id) {
  for(auto i = 0; i<1000000; i++){
    SmartPtr<int> tmp = origin1;
    *(tmp.row_ptr) = id;
  }
}

void task5(int id) {
  for(auto i = 0; i<1000000;i++) {
    shared_ptr<int> tmp = origin2;
    *tmp = id;
  }
}
/*
int main() {
  struct timeval t1,t2,t3;
  gettimeofday(&t1, nullptr);


  vector<thread> v2;
  for (auto i=0;i<100;i++)
    v2.push_back(thread(task5,i));
  for (auto i=0;i<100;i++)
    v2[i].join();

  gettimeofday(&t2, nullptr);
  cout <<
      (t2.tv_sec - t1.tv_sec)*1000*1000 +
      (t2.tv_usec - t1.tv_usec)
      << endl;

  vector<thread> v1;
  for (auto i=0;i<100;i++)
    v1.push_back(thread(task4,i));
  for (auto i=0;i<100;i++)
    v1[i].join();

  gettimeofday(&t3, nullptr);


  cout <<
      (t3.tv_sec - t2.tv_sec)*1000*1000 +
      (t3.tv_usec - t2.tv_usec)
      << endl;

}
*/
class A {
 public :
  int a;
  int b;
  A(){};
  A(int _a, int _b):a(_a),b(_b){}
  string ToString() {
    return "a:"+to_string(a)+",b:"+to_string(b);
  }
  ~A() { cout << "A Destroy"<< a << ","<< b<< endl;}
};
int tmp1 = 0;
int tmp2 = 1;
void t(int * & t1, int * t2) {
  t1 = t2;
}

class Trans{
 public:
  UInt64 TransId = 0;
  char Type = kRead;
  char Visible = kUVisible;
  atomic<int> NPart;
  atomic<int> NCommit;
  atomic<int> NAbort;
  vector<int *> StripList;
};
LockFreeList<A> list;
void task10(int id){
  for (auto i=0;i<10;i++) {
    auto ptr = list.Insert();
    ptr->getPayLoad()->a = id;
    ptr->getPayLoad()->b = id;
    sleep(1);
  }
}
int  main(){
  //LogService::Startup();
  //TranService::Startup();
  /*
  vector<pair<UInt64, UInt64>> args = {{1,22}, {2, 50}};
  Tran * tran_ptr1 = TranAPIs::Local::CreateWriteTran(args);
  Tran * tran_ptr2 = TranAPIs::Local::CreateWriteTran(args);

  TranAPIs::Local::CommitWriteTran(tran_ptr2);
  TranAPIs::Local::CommitWriteTran(tran_ptr1);
  */
  /*
  LockFreeList<A> list;
  auto ptr1 =  list.Insert();
  ptr1->PayLoad.a = 1;
  ptr1->PayLoad.b = 2;
  auto ptr2 = list.Insert();
  ptr2->PayLoad.a = 3;
  ptr2->PayLoad.b = 4;
  cout << list.ToString() << endl;
  cout << list.ToString() << endl;
*/

/*

  ptr2->PayLoad.a = 3;
  ptr2->PayLoad.b = 4;
  cout << list.ToString() << endl;
*/

/*
  auto ptr1 = list.Insert();
  ptr1->getPayLoad()->a = 1;
  ptr1->getPayLoad()->b = 1;
  cout << list.ToString() << endl;

  auto ptr2 = list.Insert();
  ptr2->getPayLoad()->a = 2;
  ptr2->getPayLoad()->b = 2;
  cout << list.ToString() << endl;
*/
  /*
  vector<thread> vt;
  for (auto i=0;i<10;i++)
    vt.push_back(thread(task10 ,i));
  for (auto i=0;i<10;i++)
    vt[i].join();
  cout << list.ToString() << endl;
  */
  {
  int * a = new int(4);
  cout << *a << endl;
  GCService::Startup();
  sleep(1);
  GCService::Collect(GCService::kInt, a);
  cout << *a << endl;
  sleep(4);
  cout << *a << endl;
  }
  cout << sizeof(Strip) << endl;
  cout << sizeof(Tran) << endl;
  vector<UInt64> partList = {1, 3};
  Snapshot snapshot;
  TranAPIs::Local::getSnapshot(partList, snapshot);
  caf::await_all_actors_done();
}


