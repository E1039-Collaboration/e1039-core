#pragma once
#include <vector>
class TH1;

int setup_server();
void handleconnection(void *arg);
void handletest(void *arg);
void send_test_message();
void receive_hist_all();
int  monitor_subsys(const char* name, std::vector<TH1*>& hist_list);
