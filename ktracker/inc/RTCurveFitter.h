#ifndef __MAKERTv7_H__
#define __MAKERTv7_H__
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <TFitterMinuit.h>
#include <Minuit2/FCNBase.h>
#include <TH2D.h>

/// Class for fitting R-T histogram
class AnaFit : public ROOT::Minuit2::FCNBase {
 protected:
  static const int N_PT = 7; ///< N of R-T points except the two edge points
  TFitterMinuit* m_minuit;
  TH2*   m_h2_RT;
  double m_t_min;
  double m_t_max;
  double m_r_max;

  double m_t1;
  double m_t0;
  double m_dr;
  TSpline3* m_spl_t2r;

 public:
  AnaFit() { m_spl_t2r = 0; }
  virtual ~AnaFit() {;}

  virtual double operator()(const std::vector<double>& r) const;
  virtual double Up() const { return 1.0; }

  void InitInput(TH2* h2, double r_max, TGraph* gr_init);
  void DoFit();
  void CalcChi2(const std::vector<double>& pars, double& chi2, int& ndf) const;

  double EvalR(const double t, const double t1, const double t0, const TSpline3* spl) const;
  double EvalR(const double t) const;

  TSpline3* GetResultSpline() const { return m_spl_t2r; }
  double    GetResultDR    () const { return m_dr     ; }
  double    GetResultT1    () const { return m_t1     ; }
  double    GetResultT0    () const { return m_t0     ; }

 private:
  TSpline3* CreateSpline(const std::vector<double>& pars) const;
};

inline double AnaFit::operator()(const std::vector<double>& pars) const
{
  double chi2;
  int ndf;
  CalcChi2(pars, chi2, ndf);
  return chi2;
}

/**
 * N of parameters = N_PT + 3
 * pars[0] = T1
 *     [1] = T0
 *     [2] = DR (constant over T)
 *     [3]...[3+N_PT-1] = R at each T points
 *
 * Note that the T position of each point is not fixed but depends on T1 & T0.
 */
inline void AnaFit::CalcChi2(const std::vector<double>& pars, double& chi2, int& ndf) const
{
   double t1 = pars[0];
   double t0 = pars[1];
   double dr = pars[2];
   TSpline3* spl = CreateSpline(pars);

   chi2 = 0;
   ndf  = 0;
   double r_width = m_h2_RT->GetYaxis()->GetBinWidth(1);
   for (int it = m_h2_RT->GetNbinsX(); it > 0; it--) {
      double cont_t  = m_h2_RT->Integral(it, it);
      if (cont_t < 10) continue;
      double t_cent = m_h2_RT->GetXaxis()->GetBinCenter(it);
      double r_func = EvalR(t_cent, t1, t0, spl);
      for (int ir = m_h2_RT->GetNbinsY(); ir > 0; ir--) {
         double cont = m_h2_RT->GetBinContent(it, ir);
         if (cont <= 0) continue;
         double r_cent  = m_h2_RT->GetYaxis()->GetBinCenter(ir);
         double cont_func = cont_t * r_width * exp( -pow( (r_cent-r_func)/dr, 2 )/2 ) / (sqrt(2*TMath::Pi()) * dr);
         double chi2p = pow(cont - cont_func, 2) / cont;
         //std::cout << "XXX " << it << " " << ir << " : " << dr << " " << cont_t << " " << cont_func << " " << cont << " " << chi2p << std::endl;
         chi2 += chi2p;
         ndf++;
      }
   }
   ndf -= m_minuit->GetNumberFreeParameters();
   delete spl;
}

inline void AnaFit::InitInput(TH2* h2, double r_max, TGraph* gr_init)
{
   //std::cout << "InitInput()" << std::endl;
   m_h2_RT = h2;
   m_r_max = r_max;
   m_t_min = h2->GetXaxis()->GetXmin();
   m_t_max = h2->GetXaxis()->GetXmax();
   m_minuit = new TFitterMinuit(N_PT+3);
   m_minuit->SetMinuitFCN(this);
   m_minuit->CreateMinimizer();
   m_minuit->SetPrintLevel(0); // Comment out this line to debug the fitting result

   std::ostringstream oss;
   int  n_pt = gr_init->GetN();
   double t1 = gr_init->GetX()[0];
   double t0 = gr_init->GetX()[n_pt-1];
   m_minuit->SetParameter(0, "T1", t1,   0.1, m_t_min, m_t_max);
   m_minuit->SetParameter(1, "T0", t0,   0.1, m_t_min, m_t_max);
   m_minuit->SetParameter(2, "DR", 0.04, 0.001, -1, -1);
   for (int i_pt = 0; i_pt < N_PT; i_pt++) {
      double t = t1 + (t0 - t1) * (i_pt + 2) / (N_PT + 2);
      double r = gr_init->Eval(t);
      oss.str("");   oss << "R of point #" << i_pt+1 << "}";
      m_minuit->SetParameter(i_pt+3, oss.str().c_str(), r, 0.01, 0, r_max);
      //std::cout << "  " << oss.str() << "   =   " << par << std::endl;
   }
}

inline void AnaFit::DoFit()
{
   //std::cout << "DoFit()" << std::endl;
   const int nfcn = 1000000; // N of max function calls per minimization
   int ret = m_minuit->Minimize(nfcn);
   if (ret != 0) {
      std::cerr << "    WARNING:  Bad fit result (ret = " << ret << ").  Please inspect it." << std::endl;
      //<< "Please fix the problem.  Abort" << std::endl;
      //exit(1);
   }
   int n_par = m_minuit->GetNumberTotalParameters();
   std::vector<double> list_par;
   for (int ii = 0; ii < n_par; ii++) list_par.push_back(m_minuit->GetParameter(ii));
   double chi2;
   int ndf;
   CalcChi2(list_par, chi2, ndf);
   std::cout << "    chi2 / ndf = " << chi2 << " / " << ndf << " = " << chi2/ndf << std::endl;

   m_t1 = m_minuit->GetParameter(0);
   m_t0 = m_minuit->GetParameter(1);
   m_dr = m_minuit->GetParameter(2);
   m_spl_t2r = CreateSpline(list_par);
}

double AnaFit::EvalR(const double t, const double t1, const double t0, const TSpline3* spl) const
{
   if      (t < t1) return m_r_max;
   else if (t > t0) return 0;
   else {
      double r = spl->Eval(t);
      if      (r > m_r_max) return m_r_max;
      else if (r < 0      ) return 0;
      else                  return r;
   }
}

double AnaFit::EvalR(const double t) const
{
   return EvalR(t, m_t1, m_t0, m_spl_t2r);
}

TSpline3* AnaFit::CreateSpline(const std::vector<double>& pars) const
{
   double t1 = pars[0];
   double t0 = pars[1];
   //double dr = pars[2];

   double array_T[N_PT+2];
   double array_R[N_PT+2];
   array_T[0] = t1;
   array_R[0] = m_r_max;
   for (int i_pt = 0; i_pt < N_PT; i_pt++) {
      double t = t1 + (t0 - t1) * (i_pt + 2) / (N_PT + 2);
      array_T[i_pt+1] = t;
      array_R[i_pt+1] = pars[3+i_pt];
   }
   array_T[N_PT+1] = t0;
   array_R[N_PT+1] = 0;
   return new TSpline3("spl", array_T, array_R, N_PT+2, "b1e1", 0, 0);
}

#endif // __MAKERTv7_H__
