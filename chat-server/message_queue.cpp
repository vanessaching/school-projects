/*
 * Implements a FIFO queue delivering messages to receivers
 * CSF Assignment 5
 * V. Ching
 * vching1@jhu.edu
 * S. Sayyad
 * ssayyad1@jhu.edu
 */
 
#include <cassert>
#include <ctime>
#include "message_queue.h"
#include "message.h"


MessageQueue::MessageQueue() {
  //initialize the mutex and the semaphore
  pthread_mutex_init(&m_lock, nullptr);
  sem_init(&m_avail, 0, 0);
}

MessageQueue::~MessageQueue() {
  //destroy the mutex and the semaphore
  pthread_mutex_lock(&m_lock);
  while (!m_messages.empty()) {
    Message *msg = m_messages.front();
    m_messages.pop_front();
    delete msg;
  }
  pthread_mutex_unlock(&m_lock);

  sem_destroy(&m_avail);
  pthread_mutex_destroy(&m_lock);
}

void MessageQueue::enqueue(Message *msg) {
  //put the specified message on the queue
  pthread_mutex_lock(&m_lock);
  m_messages.push_back(msg);
  pthread_mutex_unlock(&m_lock);

  // be sure to notify any thread waiting for a message to be
  // available
  sem_post(&m_avail);
}

Message *MessageQueue::dequeue() {
  struct timespec ts;

  // get the current time using clock_gettime:
  // we don't check the return value because the only reason
  // this call would fail is if we specify a clock that doesn't
  // exist
  clock_gettime(CLOCK_REALTIME, &ts);

  // compute a time one second in the future
  ts.tv_sec += 1;

  // call sem_timedwait to wait up to 1 second for a message
  //       to be available, return nullptr if no message is available
  if (sem_timedwait(&m_avail, &ts) == -1) {
    return nullptr;
  }

  //remove the next message from the queue, return it
  pthread_mutex_lock(&m_lock);
  assert(!m_messages.empty());
  Message *msg = m_messages.front();
  m_messages.pop_front();
  pthread_mutex_unlock(&m_lock);

  return msg;
}
