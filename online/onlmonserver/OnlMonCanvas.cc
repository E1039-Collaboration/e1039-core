#include <sstream>
#include <iomanip>
#include <TCanvas.h>
#include <TPaveText.h>
#include "OnlMonCanvas.h"
using namespace std;

OnlMonCanvas::OnlMonCanvas(const std::string name, const int num) :
  m_name(name), m_num(num),
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
  oss << m_name << " #" << m_num;
  string title = oss.str();
  m_can.SetTitle(title.c_str());

  m_can.cd();  m_pad_title.Draw();
  m_can.cd();  m_pad_main .Draw();
  m_can.cd();  m_pad_msg  .Draw();

  m_pad_title.cd();
  TPaveText* pate = new TPaveText(.02, .02, .98, .98);
  pate->AddText(title.c_str());
  pate->Draw();
}

void OnlMonCanvas::PostDraw()
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

  ostringstream oss;
  oss << "/dev/shm/" << m_name << "_" << m_num << ".png";
  m_can.SaveAs(oss.str().c_str());
}
