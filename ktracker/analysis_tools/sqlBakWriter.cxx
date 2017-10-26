#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>

#include "SRecEvent.h"
#include "JobOptsSvc.h"
#include "GeomSvc.h"
#include "MySQLSvc.h"

using namespace std;

int main(int argc, char **argv)
{
  cout << "Uploading file: " << argv[1] << " to sql schema " << argv[2] << endl;

  ///Initialize job option service
  JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();

  ///Initialize the geometry service and output file 
  GeomSvc* p_geomSvc = GeomSvc::instance();

  ///Intialize the mysql service
  MySQLSvc* p_mysqlSvc = MySQLSvc::instance();
  p_mysqlSvc->setUserPasswd("production", "qqbar2mu+mu-");
  p_mysqlSvc->connectOutput(argv[3], atoi(argv[4]));
  p_mysqlSvc->setOutputSchema(argv[2]);
  if(!p_mysqlSvc->initBakWriter()) exit(EXIT_FAILURE);

  ///Retrieve data from file
//  TClonesArray* tracklets = new TClonesArray("Tracklet");
  SRecEvent* mixEvent = new SRecEvent();
  SRecEvent* ppEvent = new SRecEvent();
  SRecEvent* mmEvent = new SRecEvent();

  TFile* dataFile = new TFile(argv[1], "READ");
  TTree* mixTree = (TTree*)dataFile->Get("save_mix");
  TTree* ppTree = (TTree*)dataFile->Get("save_pp");
  TTree* mmTree = (TTree*)dataFile->Get("save_mm");

  mixTree->SetBranchAddress("recEvent", &mixEvent);
  ppTree->SetBranchAddress("recEvent", &ppEvent);
  mmTree->SetBranchAddress("recEvent", &mmEvent);
//  dataTree->SetBranchAddress("tracklets", &tracklets);

  int nEventsMix = mixTree->GetEntries();
  for(int i = 0; i < nEventsMix; ++i)
    {
      mixTree->GetEntry(i);
      cout << "\r Uploading event " << mixEvent->getEventID() << ", " << (i+1)*100/nEventsMix << "% finished." << flush;
      p_mysqlSvc->writeTrackingBak(mixEvent, "Mix");
    }
  cout << endl;
  cout << "Uploaded Mix events successfully." << endl;

  int nEventsPP = ppTree->GetEntries();
  for(int i = 0; i < nEventsPP; ++i)
    {
      ppTree->GetEntry(i);
      cout << "\r Uploading event " << ppEvent->getEventID() << ", " << (i+1)*100/nEventsPP << "% finished." << flush;
      p_mysqlSvc->writeTrackingBak(ppEvent, "PP");
    }
  cout << endl;
  cout << "Uploaded PP events successfully." << endl;

  int nEventsMM = mmTree->GetEntries();
  for(int i = 0; i < nEventsMM; ++i)
    {
      mmTree->GetEntry(i);
      cout << "\r Uploading event " << mmEvent->getEventID() << ", " << (i+1)*100/nEventsMM << "% finished." << flush;
      p_mysqlSvc->writeTrackingBak(mmEvent, "MM");
    }
  cout << endl;
  cout << "Uploaded MM events successfully." << endl;
  cout << "sqlResWriter ends successfully." << endl;

  delete p_mysqlSvc;
  delete p_geomSvc;
  delete p_jobOptsSvc;

  return EXIT_SUCCESS;
}
