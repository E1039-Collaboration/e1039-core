
R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libktthread)


int RunScheduler(TString infile = "input.root", TString outfile = "output.root")
{
    // Enable thread safety feature from ROOT
    std::cout << "ROOT enabling thread safety..." << std::endl;
    ROOT::EnableThreadSafety();
    //gSystem->Load("/home/Tracking/kThreadMerge/core-inst/lib/libktthread.so");
    std::cout << "Loading ROOT and system pthreads..." << std::endl;
    gSystem->Load("/usr/lib64/libpthread.so");
    gSystem->Load("$ROOTSYS/lib/libThread.so");

    std::cout << "Start of kTThreadTest" << std::endl;

    // Initialize running constants
    const bool cosmic = true;
    const double FMAGSTR = -1.054;
    const double KMAGSTR = -0.951;

    recoConsts* rc = recoConsts::instance();
    rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
    rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
    if(cosmic)
    {
        rc->init("cosmic");
        rc->set_BoolFlag("COARSE_MODE", true);
        rc->set_DoubleFlag("KMAGSTR", 0.);
        rc->set_DoubleFlag("FMAGSTR", 0.);
    }
    rc->Print();

    // Initialize geometry
    GeomSvc::UseDbSvc(true);  
    GeomSvc* geom_svc = GeomSvc::instance();

    // Start the scheduler call
    std::cout << "Initializing Scheduler" << std::endl;
    // test with the following files...
    KScheduler* ksched = new KScheduler("/home/Tracking/2960_test.root","/home/Tracking/parallel_test.root");
//    ksched->setInputFilename(infile);
//    ksched->setOutputFilename(outfile);

    std::cout << "Master call to run the threads and join" << std::endl;
    ksched->runThreads();

    std::cout << "All threads dead! all done... bye bye!\n";

    delete ksched;
    return 0;
}


