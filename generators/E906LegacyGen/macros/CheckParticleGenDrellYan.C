R__LOAD_LIBRARY(libSQPrimaryGen)
using namespace std;

/// Macro to check the cross-section value of D-Y computed by SQPrimaryParticleGen.
int CheckParticleGenDrellYan()
{
  const double xF       = 0.3;
  const int    n_mass   = 20;
  const double mass_min = 3.0;
  const double mass_max = 9.0;
  const double pARatio  = 1.0; // p+p
  const char* fn_plot   = "xsec_gen_dy.png";
  const char* fn_text   = "xsec_gen_dy.txt";

  TGraph* gr = new TGraph();
  ofstream ofs(fn_text);

  SQPrimaryParticleGen* gen = new SQPrimaryParticleGen();
  gen->enableDrellYanGen();
  gen->set_pT0DY(0); // Keep pT = 0.  Otherwise x1, x2 and thus xsec vary.

  for (int i_mass = 0; i_mass < n_mass; i_mass++) {
    double mass = mass_min + (mass_max - mass_min) * i_mass / (n_mass-1);
    double xsec = gen->CrossSectionDrellYan(mass, xF, pARatio) * pow(mass, 3);
    gr->SetPoint(i_mass, mass, xsec);
    ofs << mass << "\t" << xsec << "\n";
  }

  ofs.close();

  ostringstream oss;
  oss << "Drell-Yan @ x_{F} = " << xF << ";Mass (GeV);M^{3} d^{2}#sigma/dMdx_{F} (pb GeV^{2})";
  gr->SetTitle(oss.str().c_str());
  gr->SetLineWidth(2);
  gr->SetMarkerStyle(8);

  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetLogy();
  c1->SetGrid();
  gr->Draw("APC");
  c1->SaveAs(fn_plot);

  return 0;
}
