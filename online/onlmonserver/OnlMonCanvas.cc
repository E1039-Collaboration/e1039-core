#include <sstream>
#include <iomanip>
#include <TSystem.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include "OnlMonCanvas.h"
using namespace std;

OnlMonCanvas::OnlMonCanvas(const std::string name, const std::string title, const int num, const int run) :
  m_name(name), m_title(title), m_num(num), m_run(run),
  m_can("c1", "", 5+600*num, 5, 600, 800), 
  m_pad_title("title", "", 0.0, 0.9, 1.0, 1.0),
  m_pad_main ("main" , "", 0.0, 0.1, 1.0, 0.9),
  m_pad_msg  ("msg"  , "", 0.0, 0.0, 1.0, 0.1),
  m_pate_msg(.02, .02, .98, .98),
  m_mon_status(UNDEF)
{
  //m_can.SetWindowPosition(5+600*num, 5);
}

OnlMonCanvas::~OnlMonCanvas()
{
  ;
}

void OnlMonCanvas::AddMessage(const char* msg)
{
  m_pate_msg.AddText(msg);
}

TPad* OnlMonCanvas::GetMainPad()
{
  m_pad_main.cd();
  return &m_pad_main; 
}

void OnlMonCanvas::PreDraw()
{
  ostringstream oss;
  oss << m_name << " Canvas #" << m_num;
  m_can.SetTitle(oss.str().c_str());

  m_can.cd();  m_pad_title.Draw();
  m_can.cd();  m_pad_main .Draw();
  m_can.cd();  m_pad_msg  .Draw();

  m_pad_title.cd();
  TPaveText* pate = new TPaveText(.02, .52, .98, .98);
  oss.str("");
  oss << m_title << " #" << m_num;
  pate->AddText(oss.str().c_str());
  pate->Draw();

  time_t utime = time(0);
  char stime[64];
  strftime(stime, sizeof(stime),"%Y-%m-%d %H:%M:%S", localtime(&utime));
  oss.str("");
  oss << "Run " << m_run << " : Drawn at " << stime;

  TPaveText* pate2 = new TPaveText(.02, .02, .98, .48, "NB");
  pate2->SetFillColor(kWhite);
  pate2->AddText(oss.str().c_str());
  pate2->Draw();
}

void OnlMonCanvas::PostDraw(const bool at_end)
{
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

  if (at_end) {
    ostringstream oss;
    oss << "/dev/shm/onlmon/" << setfill('0') << setw(6) << m_run;
    gSystem->mkdir(oss.str().c_str(), true);

    int lvl = gErrorIgnoreLevel;
    gErrorIgnoreLevel = 1111; // Suppress the message by TCanvas::SaveAs().
    oss << "/" << m_name << "_can" << m_num << ".png";
    m_can.SaveAs(oss.str().c_str());
    gErrorIgnoreLevel = lvl;
  }
}
