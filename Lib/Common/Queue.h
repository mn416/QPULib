#ifndef _QUEUE_H_
#define _QUEUE_H_

namespace QPULib {

  // Very simple queue containing N elements of type T
  template <int N, typename T> struct Queue {
    T elems[N+1];
    int front;
    int back;
    Queue() { front = back = 0; }
    bool isEmpty() { return front == back; }
    bool isFull() { return ((back+1)%(N+1)) == front; }
    void enq(T elem) { elems[back] = elem; back = (back+1)%(N+1); }
    T* first() { return &elems[front]; }
    void deq() { front = (front+1)%(N+1); }
  };

}

#endif
