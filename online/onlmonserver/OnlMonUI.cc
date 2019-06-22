#include <iostream>
#include "OnlMonClient.h"
#include "OnlMonUI.h"
using namespace std;

OnlMonUI::OnlMonUI(OnlMonClientList_t* list) :
  m_auto_cycle(false), m_interval(10), m_thread_id(0)
{
  for (OnlMonClientList_t::iterator it = list->begin(); it != list->end(); it++) {
    m_list_omc.push_back((*it)->Clone());
  }
}

void OnlMonUI::StartAutoCycle()
{
  pthread_create(&m_thread_id, NULL, FuncAutoCycle, this);
}

void* OnlMonUI::FuncAutoCycle(void* arg)
{
  OnlMonUI* ui = (OnlMonUI*)arg;
  ui->RunAutoCycle();
}

void OnlMonUI::RunAutoCycle()
{
  int idx = 0;
  while (true) {
    if (m_auto_cycle) {
      m_list_omc[idx]->StartMonitor();
      if (++idx >= m_list_omc.size()) idx = 0;
    }
    sleep(m_interval);
  }
}
