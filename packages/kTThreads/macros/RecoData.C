/// ROOT macro to run the online reconstruction.
/** 
 * Based on `RunScheduler.C`.
 */
R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libktthread)

int RecoData(const int run_id, TString infile="input.root", TString outfile="output.root")
{
    // Enable thread safety feature from ROOT
    std::cout << "ROOT enabling thread safety..." << std::endl;
    ROOT::EnableThreadSafety();
    std::cout << "Start of kTThreadTest" << std::endl;

    // Initialize running constants
    const bool cosmic = true;
    recoConsts* rc = recoConsts::instance();
    rc->set_IntFlag("RUNNUMBER", run_id);
    rc->set_CharFlag("EventReduceOpts", "a"); // Better set "none" if possible
    if (cosmic) {
        rc->init("cosmic");
        rc->set_BoolFlag("COARSE_MODE", true);
        rc->set_DoubleFlag("KMAGSTR", 0.);
        rc->set_DoubleFlag("FMAGSTR", 0.);
    } else {
      const double FMAGSTR = -1.054;
      const double KMAGSTR = -0.951;
      rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
      rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
    }
    rc->Print();

    // Start the scheduler call
    std::cout << "Initializing Scheduler" << std::endl;
    //KJob      ::Verbose(1);
    //KScheduler::Verbose(1);
    KScheduler* ksched = new KScheduler(infile, outfile);
    ksched->UseTrackletReco(true);
    ksched->Init();

    std::cout << "Master call to run the threads and join" << std::endl;
    ksched->runThreads();

    std::cout << "All threads dead! all done... bye bye!\n";

    delete ksched;
    return 0;
}


