R__LOAD_LIBRARY(libSQPrimaryGen)
using namespace std;

/// Macro to check the cross-section value of J/psi computed by SQPrimaryParticleGen.
int CheckParticleGenJPsi()
{
  const int    n_xF   = 11;
  const double xF_min = -0.1;
  const double xF_max =  0.9;
  const char* fn_plot = "xsec_gen_jpsi.png";
  const char* fn_text = "xsec_gen_jpsi.txt";

  TGraph* gr = new TGraph();
  ofstream ofs(fn_text);

  SQPrimaryParticleGen* gen = new SQPrimaryParticleGen();
  gen->enableJPsiGen();

  for (int i_xF = 0; i_xF < n_xF; i_xF++) {
    double xF = xF_min + (xF_max - xF_min) * i_xF / (n_xF-1);
    double xsec = gen->CrossSectionJPsi(xF);
    gr->SetPoint(i_xF, xF, xsec);
    ofs << xF << "\t" << xsec << "\n";
  }

  ofs.close();

  gr->SetTitle("J/#psi #rightarrow #mu^{+}#mu^{-};x_{F};BR d#sigma/dx_{F} (pb)");
  gr->SetLineWidth(2);
  gr->SetMarkerStyle(8);

  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetLogy();
  c1->SetGrid();
  gr->Draw("APC");
  c1->SaveAs(fn_plot);

  return 0;
}
