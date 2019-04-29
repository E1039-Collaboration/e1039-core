#include <iomanip>
#include <TH1D.h>
#include <TSocket.h>
#include <TClass.h>
#include <TMessage.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllHistoManager.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include "OnlMonServer.h"
#include "OnlMonCanvas.h"
#include "OnlMonClient.h"
using namespace std;

OnlMonClient::OnlMonClient(const std::string &name) : SubsysReco(name)
{
  m_n_can = 1;
  memset(m_list_can, 0, sizeof(m_list_can));
}

OnlMonClient::~OnlMonClient()
{
  ClearHistList();
}

TH1* OnlMonClient::FindMonHist(const std::string name, const bool non_null)
{
  for (HistList_t::iterator it = m_list_h1.begin(); it != m_list_h1.end(); it++) {
    if (name == (*it)->GetName()) return *it;
  }
  if (non_null) {
    cerr << "!!ERROR!!  OnlMonClient::FindMonHist() cannot find '" << name << "'.  Abort." << endl;
    exit(1);
  }
  return 0;
}

TObject* OnlMonClient::FindMonObj(const std::string name, const bool non_null)
{
  for (ObjList_t::iterator it = m_list_obj.begin(); it != m_list_obj.end(); it++) {
    if (name == (*it)->GetName()) return *it;
  }
  if (non_null) {
    cerr << "!!ERROR!!  OnlMonClient::FindMonObj() cannot find '" << name << "'.  Abort." << endl;
    exit(1);
  }
  return 0;
}

int OnlMonClient::DrawMonitor()
{
  cerr << "!!ERROR!!  OnlMonClient::DrawMonitor(): virtual function called.  Abort." << endl;
  exit(1);
}

int OnlMonClient::StartMonitor()
{
  for (int ii = 0; ii < m_n_can; ii++) {
    if (m_list_can[ii]) delete m_list_can[ii];
  }

  ReceiveHist();

  for (int ii = 0; ii < m_n_can; ii++) {
    m_list_can[ii] = new OnlMonCanvas(Name(), ii);
    m_list_can[ii]->PreDraw();
  }

  int ret = DrawMonitor();

  for (int ii = 0; ii < m_n_can; ii++) {
    m_list_can[ii]->PostDraw();
  }

  return ret;
}

int OnlMonClient::ReceiveHist()
{
  const char* ONL_MON_SVR = "localhost";
  const int ONL_MON_PORT = 9081;
  string comm = "SUBSYS:";
  comm += Name();

  cout << "OnlMonClient::ReceiveHist(): "
       << ONL_MON_SVR << ":" << ONL_MON_PORT << " " << comm << endl;
  TSocket sock(ONL_MON_SVR, ONL_MON_PORT);
  sock.Send(comm.c_str());

  ClearHistList();

  TMessage *mess = NULL;
  while (true) { // incoming hist
    sock.Recv(mess);
    if (!mess) {
      break;
    } else if (mess->What() == kMESS_STRING) {
      char str[200];
      mess->ReadString(str, 200);
      delete mess;
      mess = 0;
      if (!strcmp(str, "Finished")) break;
    } else if (mess->What() == kMESS_OBJECT) {
      TClass*  cla = mess->GetClass();
      TObject* obj = mess->ReadObject(cla);
      cout << "  Receive: " << cla->GetName() << " " << obj->GetName() << endl;
      m_list_obj.push_back( obj->Clone() ); // copy

      //TH1*    obj = (TH1*)mess->ReadObject(cla);
      //cout << "  Receive: " << cla->GetName() << " " << obj->GetName() << endl;
      //m_list_h1.push_back( (TH1*)obj->Clone() ); // copy
      delete mess;
      mess = 0;
      sock.Send("NEXT"); // Any text is ok for now
    }
  }
  sock.Close();
  return 0;
}

void OnlMonClient::ClearHistList()
{
  for (HistList_t::iterator it = m_list_h1.begin(); it != m_list_h1.end(); it++) {
    delete *it;
  }
  m_list_h1.clear();

  //for (ObjList_t::iterator it = m_list_obj.begin(); it != m_list_obj.end(); it++) {
  //  delete *it;
  //}
  //m_list_obj.clear();
}
OnlMonCanvas* OnlMonClient::GetCanvas(const int num) 
{
  if (num >= m_n_can) {
    cerr << "ERROR  OnlMonClient::GetCanvas():  Num out of range (" << num << " >= " << m_n_can << ").  Abort.";
    exit(1);
  }
  return m_list_can[num]; 
}
