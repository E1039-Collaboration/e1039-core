#include <iostream>
#include <fstream>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TCanvas.h>

using namespace std;

int main(int argc, char* argv[])
{
    int hodoID;
    int elementID;
    int flag;

    /*
    int nElements[8] = {20, 20, 19, 19, 16, 16, 16, 16};
    int hodoIDs[8] = {27, 28, 29, 30, 35, 36, 37, 38};
    std::string hodoNames[8] = {"H1L", "H1R", "H2L", "H2R", "H4Y1L", "H4Y1R", "H4Y2L", "H4Y2R"};
    TFile* dataFile = new TFile("hodoeff_Y.root", "READ");
    */

    int nElements[8] = {23, 23, 16, 16, 16, 16, 16, 16};
    int hodoIDs[8] = {25, 26, 31, 32, 33, 34, 39, 40};
    std::string hodoNames[8] = {"H1B", "H1T", "H2B", "H2T", "H3B", "H3T", "H4B", "H4T"};

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    dataTree->SetBranchAddress("hodoID", &hodoID);
    dataTree->SetBranchAddress("elementID", &elementID);
    dataTree->SetBranchAddress("flag", &flag);

    TH1I* hist_all[8];
    TH1I* hist_acc[8];
    TH1D* hist_eff[8];
    char buffer[20];
    for(int i = 0; i < 8; ++i)
    {
        sprintf(buffer, "%s_all", hodoNames[i].c_str());
        hist_all[i] = new TH1I(buffer, buffer, nElements[i], 1, nElements[i]+1);
        hist_all[i]->Sumw2();

        sprintf(buffer, "%s_acc", hodoNames[i].c_str());
        hist_acc[i] = new TH1I(buffer, buffer, nElements[i], 1, nElements[i]+1);
        hist_acc[i]->Sumw2();

        sprintf(buffer, "%s", hodoNames[i].c_str());
        hist_eff[i] = new TH1D(buffer, buffer, nElements[i], 1, nElements[i]+1);
        hist_eff[i]->Sumw2();

        hist_eff[i]->GetXaxis()->SetTitle("elementID");
        hist_eff[i]->GetXaxis()->CenterTitle();
        hist_eff[i]->SetMarkerStyle(8);
        hist_eff[i]->SetMarkerSize(0.4);
    }

    for(int i = 0; i < dataTree->GetEntries(); ++i)
    {
        dataTree->GetEntry(i);

        int idx = -1;
        for(int j = 0; j < 8; ++j)
        {
            if(hodoIDs[j] == hodoID)
            {
                idx = j;
                break;
            }
        }

        if(idx >= 0 && idx < 8)
        {
            hist_all[idx]->Fill(elementID);
            if(flag == 1) hist_acc[idx]->Fill(elementID);
        }
    }

    for(int i = 0; i < 8; ++i)
    {
        hist_eff[i]->Divide(hist_acc[i], hist_all[i], 1., 1., "B");
    }

    TCanvas* c1 = new TCanvas();
    c1->Divide(4, 2);
    c1->SetGridx();
    c1->SetGridy();

    ofstream fout(argv[2]);
    for(int i = 1; i <= 8; ++i)
    {
        c1->cd(i)->SetGridx();
        c1->cd(i)->SetGridy();
        //c1->cd(i)->SetLogy();
        //hist_all[i-1]->Draw(); hist_acc[i-1]->Draw("same");
        hist_eff[i-1]->GetYaxis()->SetRangeUser(0.0, 1.3);
        hist_eff[i-1]->Draw();

        for(int j = 1; j <= hist_eff[i-1]->GetNbinsX(); ++j)
        {
            fout << hist_eff[i-1]->GetTitle() << "  " << j << "  " << hist_all[i-1]->GetBinContent(j) << "  " << hist_eff[i-1]->GetBinContent(j) << " +/- " << hist_eff[i-1]->GetBinError(j) << endl;
        }
    }
    fout.close();

    c1->SaveAs(argv[3]);

    return 1;
}
