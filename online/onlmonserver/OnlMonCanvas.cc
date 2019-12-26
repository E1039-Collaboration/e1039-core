#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <TROOT.h>
#include <TError.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRootCanvas.h>
#include <TPaveText.h>
#include <UtilAna/UtilOnline.h>
#include "OnlMonServer.h"
#include "OnlMonCanvas.h"
using namespace std;

OnlMonCanvas::OnlMonCanvas(const std::string name, const std::string title, const int num) :
  m_name(name), m_title(title), m_num(num), 
  m_can("c1", "", 200+600*num, 20, 600, 800), 
  m_pad_title("title", "", 0.0, 0.9, 1.0, 1.0),
  m_pad_main ("main" , "", 0.0, 0.1, 1.0, 0.9),
  m_pad_msg  ("msg"  , "", 0.0, 0.0, 1.0, 0.1),
  m_pate_msg(.02, .02, .98, .98),
  m_mon_status(UNDEF),
  m_run(0), m_spill(0), m_event(0), m_n_evt(0)
{
  //m_can.SetWindowPosition(5+600*num, 5);
  //TRootCanvas *rc = dynamic_cast<TRootCanvas*>(m_can.GetCanvasImp());
  ////rc->Connect("CloseWindow()", 0, 0, "OnlMonCanvas::close(void)");
  //rc->DontCallClose();
  //rc->UnmapWindow();
  //m_can.Update();
}

OnlMonCanvas::~OnlMonCanvas()
{
  ;
}

void OnlMonCanvas::SetBasicInfo(const int run_id, const int spill_id, const int event_id, const int n_evt)
{
  m_run   = run_id;
  m_spill = spill_id;
  m_event = event_id;
  m_n_evt = n_evt;
}

void OnlMonCanvas::AddMessage(const char* msg)
{
  m_pate_msg.AddText(msg);
}


void OnlMonCanvas::SetWorseStatus(const MonStatus_t stat)
{
  switch (stat) {
  case OK:
    if (m_mon_status == UNDEF) m_mon_status = OK;
    break;
  case WARN:  
    if (m_mon_status == UNDEF || m_mon_status == OK) m_mon_status = WARN;
    break;
  case ERROR:
    m_mon_status = ERROR;
    break;
  case UNDEF:
    m_mon_status = UNDEF;
    break;
  }
}

TPad* OnlMonCanvas::GetMainPad()
{
  m_pad_main.cd();
  return &m_pad_main; 
}

void OnlMonCanvas::InitDraw()
{
  TRootCanvas *rc = dynamic_cast<TRootCanvas*>(m_can.GetCanvasImp());
  //rc->Connect("CloseWindow()", 0, 0, "OnlMonCanvas::close(void)");
  rc->DontCallClose();
  rc->UnmapWindow();

  ostringstream oss;
  oss << m_name << " Canvas #" << m_num;
  m_can.SetTitle(oss.str().c_str());

  m_can.cd();  m_pad_title.Draw();
  m_can.cd();  m_pad_main .Draw();
  m_can.cd();  m_pad_msg  .Draw();

  //Clear();
  //m_can.Update();

  m_can.Update();
}

void OnlMonCanvas::PreDraw(const bool at_end)
{
  ostringstream oss;
  //oss << m_name << " Canvas #" << m_num;
  //m_can.SetTitle(oss.str().c_str());
  //
  //m_can.cd();  m_pad_title.Draw();
  //m_can.cd();  m_pad_main .Draw();
  //m_can.cd();  m_pad_msg  .Draw();

  m_pad_title.cd();
  TPaveText* pate = new TPaveText(.02, .52, .98, .98);
  oss.str("");
  oss << m_title << ": C" << m_num;
  pate->AddText(oss.str().c_str());
  pate->Draw();

  TPaveText* pate2 = new TPaveText(.02, .02, .98, .48, "NB");
  pate2->SetFillColor(kWhite);

  oss.str("");
  oss << "Run #" << m_run;
  if (! at_end) oss << ", Spill #" << m_spill << ", Event #" << m_event << ", " << m_n_evt << " events";
  pate2->AddText(oss.str().c_str());

  time_t utime = time(0);
  char stime[64];
  strftime(stime, sizeof(stime),"%Y-%m-%d %H:%M:%S", localtime(&utime));
  oss.str("");  oss << "Drawn at " << stime;
  pate2->AddText(oss.str().c_str());

  pate2->Draw();
  gStyle->SetOptStat(0000);
  gStyle->SetHistMinimumZero(true);
}

void OnlMonCanvas::PostDraw(const bool at_end)
{
  if (m_pate_msg.GetListOfLines()->GetSize() == 0) { // Set minimum message.
    switch (m_mon_status) {
    case OK   :  m_pate_msg.AddText("OK"       );  break;
    case WARN :  m_pate_msg.AddText("Warning"  );  break;
    case ERROR:  m_pate_msg.AddText("Error"    );  break;
    default   :  m_pate_msg.AddText("Undefined");  break;
    }
  }
  int color;
  switch (m_mon_status) {
  case OK   :  color = kGreen ;  break;
  case WARN :  color = kYellow;  break;
  case ERROR:  color = kRed   ;  break;
  default   :  color = kGray  ;  break;
  }
  m_pate_msg.SetFillColor(color);
  m_pad_msg .cd();
  m_pate_msg.Draw();

  dynamic_cast<TRootCanvas*>(m_can.GetCanvasImp())->MapWindow();
  m_can.Update();

  if (at_end) {
    ostringstream oss;
    oss << UtilOnline::GetOnlMonDir() << "/" << setfill('0') << setw(6) << m_run;
    gSystem->mkdir(oss.str().c_str(), true);

    oss << "/" << m_name << "_can" << m_num;
    string path_base = oss.str(); // path without suffix

    int lvl = gErrorIgnoreLevel;
    gErrorIgnoreLevel = 1111; // Suppress the message by TCanvas::SaveAs().
    oss.str("");
    oss << path_base << ".png";
    m_can.SaveAs(oss.str().c_str());
    gErrorIgnoreLevel = lvl;

    oss.str("");
    oss << path_base << ".txt";
    ofstream ofs(oss.str().c_str());
    ofs << m_mon_status << endl;
    ofs.close();
  }
}

void OnlMonCanvas::Clear()
{
  m_pad_title.Clear();
  m_pad_main .Clear();
  m_pad_msg  .Clear();
  m_pate_msg .Clear();

  dynamic_cast<TRootCanvas*>(m_can.GetCanvasImp())->UnmapWindow();
  m_can.Update();
}
