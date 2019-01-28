
#include "PatternDBUtil.h"

#include <TTree.h>
#include <TFile.h>

#define LogInfo(message) std::cout << "DEBUG: " << __FILE__ << "  " << __LINE__ << "  " << __FUNCTION__ << " :::  " << message << std::endl

#define _DEBUG_

//#define _MULTI_KEY_

int PatternDBUtil::verbosity = 0;


std::map<unsigned int, unsigned int> PatternDBUtil::_detid_view = {
		{9,  0},
		{10, 1},
		{11, 2},
		{12, 3},
		{7,  4},
		{8,  5},

		{16, 0},
		{15, 1},
		{17, 2},
		{18, 3},
		{13, 4},
		{14, 5},

		{22, 0},
		{21, 1},
		{23, 2},
		{24, 3},
		{20, 4},
		{19, 5},

		{28, 0},
		{27, 1},
		{30, 2},
		{29, 3},
		{26, 4},
		{25, 5}
};

int PatternDBUtil::BuildPatternDB(const std::string &fin, const std::string & fout) {

#ifdef _DEBUG_
	TFile *feval = TFile::Open("PatternDBUtilEval.root","recreate");
	TTree *Teval = new TTree("T","PatternDBUtilEval");
	int ntrack = 0;
	float acc_prob_1 = 0;
	float acc_prob_2 = 0;
	float acc_prob_3 = 0;
	float acc_prob_23 = 0;
	float acc_prob_123 = 0;
	Teval->Branch("ntrack",   &ntrack,   "ntrack/I");
	Teval->Branch("acc_prob_1", &acc_prob_1, "acc_prob_1/F");
	Teval->Branch("acc_prob_2", &acc_prob_2, "acc_prob_2/F");
	Teval->Branch("acc_prob_3", &acc_prob_3, "acc_prob_3/F");
	Teval->Branch("acc_prob_23", &acc_prob_23, "acc_prob_23/F");
	Teval->Branch("acc_prob_123", &acc_prob_123, "acc_prob_123/F");

	int nacc_1 = 0;
	int nacc_2 = 0;
	int nacc_3 = 0;
	int nacc_23 = 0;
	int nacc_123 = 0;
	const int interval = 1000;
#endif

	if(verbosity >= 2) {
		LogInfo("PatternDBUtil::BuildPatternDB from " << fin);
	}

	TFile *f_in = TFile::Open(fin.c_str(), "read");
	if(!f_in) {
		LogInfo(fin << "not found!");
		return -1;
	}

	TTree *T = (TTree*) f_in->Get("T");
	if(!T) {
		LogInfo("TTree T not found in " << fin);
		return -1;
	}

	int n_particles = 0;
	int elmid [1000][55];

	T->SetBranchAddress("n_particles", &n_particles);
	T->SetBranchAddress("gelmid", &elmid);

	PatternDB db;

#ifdef _DEBUG_
	ntrack = 0;
	nacc_1 = 0;
	nacc_2 = 0;
	nacc_3 = 0;
	nacc_23 = 0;
	nacc_123 = 0;
#endif

	for(int ientry=0;ientry<T->GetEntries();++ientry) {
	//for(int ientry=0;ientry<10000;++ientry) {
		T->GetEntry(ientry);

		for(int ipar=0; ipar<n_particles; ++ipar) {

			unsigned int D1V  = elmid[ipar][7];
			unsigned int D1Vp = elmid[ipar][8];
			unsigned int D1X  = elmid[ipar][9];
			unsigned int D1Xp = elmid[ipar][10];
			unsigned int D1U  = elmid[ipar][11];
			unsigned int D1Up = elmid[ipar][12];

			unsigned int D2V  = elmid[ipar][13];
			unsigned int D2Vp = elmid[ipar][14];
			unsigned int D2Xp = elmid[ipar][15];
			unsigned int D2X  = elmid[ipar][16];
			unsigned int D2U  = elmid[ipar][17];
			unsigned int D2Up = elmid[ipar][18];

			unsigned int D3pVp  = elmid[ipar][19];
			unsigned int D3pV   = elmid[ipar][20];
			unsigned int D3pXp  = elmid[ipar][21];
			unsigned int D3pX   = elmid[ipar][22];
			unsigned int D3pUp  = elmid[ipar][23];
			unsigned int D3pU   = elmid[ipar][24];

			unsigned int D3mVp  = elmid[ipar][25];
			unsigned int D3mV   = elmid[ipar][26];
			unsigned int D3mXp  = elmid[ipar][27];
			unsigned int D3mX   = elmid[ipar][28];
			unsigned int D3mUp  = elmid[ipar][29];
			unsigned int D3mU   = elmid[ipar][30];


#ifdef _DEBUG_
			auto size_1   = db.St1.size();
			auto size_2   = db.St2.size();
			auto size_3   = db.St3.size();
			auto size_23  = db.St23.size();
			auto size_123 = db.St123.size();
#endif

#ifdef _MULTI_KEY_
			// Multi key
			for (int iX1 = 0; iX1<1; ++iX1 ) {
				for (int iU1 = 0; iU1<1; ++iU1 ) {
					for (int iV1 = 0; iV1<1; ++iV1 ) {
						for (int iX2 = -1; iX2<2; ++iX2 ) {
							for (int iU2 = -1; iU2<2; ++iU2 ) {
								for (int iV2 = -1; iV2<2; ++iV2 ) {
									for (int iX3 = -1; iX3<2; ++iX3 ) {
										for (int iU3 = -1; iU3<2; ++iU3 ) {
											for (int iV3 = -1; iV3<2; ++iV3 ) {
												TrackletKey key1  = EncodeTrackletKey(PatternDB::DC1,  D1X +iX1, D1Xp,  D1U +iU1, D1Up,  D1V +iV1, D1Vp);
												TrackletKey key2  = EncodeTrackletKey(PatternDB::DC2,  D2X +iX2, D2Xp,  D2U +iU2, D2Up,  D2V +iV2, D2Vp);
												TrackletKey key3p = EncodeTrackletKey(PatternDB::DC3p, D3pX+iX3, D3pXp, D3pU+iU3, D3pUp, D3pV+iV3, D3pVp);
												TrackletKey key3m = EncodeTrackletKey(PatternDB::DC3m, D3mX+iX3, D3mXp, D3mU+iU3, D3mUp, D3mV+iV3, D3mVp);

												if(key1  != PatternDB::ERR_KEY) db.St1.insert(key1);
												if(key2  != PatternDB::ERR_KEY) db.St2.insert(key2);
												if(key3p != PatternDB::ERR_KEY) db.St3.insert(key3p);
												if(key3m != PatternDB::ERR_KEY) db.St3.insert(key3m);

												if(key2  != PatternDB::ERR_KEY and key3p != PatternDB::ERR_KEY) {
													//db_st23.insert(std::make_tuple(key2,key3p));
													db.St23.insert(PartTrackKey(key2,key3p));
													if(key1 != PatternDB::ERR_KEY) {
														db.St123.insert(GlobTrackKey(key1,key2,key3p));
													}
												}

												if(key2  != PatternDB::ERR_KEY and key3m != PatternDB::ERR_KEY) {
													//db_st23.insert(std::make_tuple(key2,key3m));
													db.St23.insert(PartTrackKey(key2,key3m));
													if(key1 != PatternDB::ERR_KEY) {
														db.St123.insert(GlobTrackKey(key1,key2,key3m));
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
#else
			// Single key
			TrackletKey key1  = EncodeTrackletKey(PatternDB::DC1, D1X, D1Xp, D1U, D1Up, D1V, D1Vp);
			TrackletKey key2  = EncodeTrackletKey(PatternDB::DC2, D2X, D2Xp, D2U, D2Up, D2V, D2Vp);
			TrackletKey key3p = EncodeTrackletKey(PatternDB::DC3p, D3pX, D3pXp, D3pU, D3pUp, D3pV, D3pVp);
			TrackletKey key3m = EncodeTrackletKey(PatternDB::DC3m, D3mX, D3mXp, D3mU, D3mUp, D3mV, D3mVp);

			if(key1  != PatternDB::ERR_KEY) db.St1.insert(key1);
			if(key2  != PatternDB::ERR_KEY) db.St2.insert(key2);
			if(key3p != PatternDB::ERR_KEY) db.St3.insert(key3p);
			if(key3m != PatternDB::ERR_KEY) db.St3.insert(key3m);

			if(key2  != PatternDB::ERR_KEY and key3p != PatternDB::ERR_KEY) {
				db.St23.insert(PartTrackKey(key2,key3p));
				if(key1 != PatternDB::ERR_KEY) {
					db.St123.insert(GlobTrackKey(key1,key2,key3p));
				}
			}

			if(key2  != PatternDB::ERR_KEY and key3m != PatternDB::ERR_KEY) {
					db.St23.insert(PartTrackKey(key2,key3m));
					if(key1 != PatternDB::ERR_KEY) {
						db.St123.insert(GlobTrackKey(key1,key2,key3m));
					}
			}
#endif

#ifdef _DEBUG_

			if(db.St1.size()>size_1) ++nacc_1;
			if(db.St2.size()>size_2) ++nacc_2;
			if(db.St3.size()>size_3) ++nacc_3;
			if(db.St23.size()>size_23) ++nacc_23;
			if(db.St123.size()>size_123) ++nacc_123;

			if(ntrack%interval==0) {
				acc_prob_1 = 1.*nacc_1/interval;
				acc_prob_2 = 1.*nacc_2/interval;
				acc_prob_3 = 1.*nacc_3/interval;
				acc_prob_23 = 1.*nacc_23/interval;
				acc_prob_123 = 1.*nacc_123/interval;

				Teval->Fill();

	      nacc_1 = 0;
	      nacc_2 = 0;
	      nacc_3 = 0;
	      nacc_23 = 0;
	      nacc_123 = 0;

				LogInfo("ntrack: "<< ntrack);
				LogInfo("acc_prob_1: "<< acc_prob_1);
				LogInfo("acc_prob_2: "<< acc_prob_2);
				LogInfo("acc_prob_3: "<< acc_prob_3);
				LogInfo("acc_prob_23: "<< acc_prob_23);
				LogInfo("acc_prob_123: "<< acc_prob_123);
			}
			++ntrack;
#endif
		}
	}

	if(verbosity >= 2) {
		LogInfo("PatternDBUtil::BuildPatternDB from " << fin);
		LogInfo("db_st23.size(): " << db.St23.size());
		db.print();
	}

	TFile *f_out = TFile::Open(fout.c_str(), "recreate");
	f_out->cd();
	db.Write();
	f_out->Close();

#ifdef _DEBUG_
	feval->cd();
	Teval->Write();
	feval->Close();
#endif
	return 0;
}

PatternDB* PatternDBUtil::LoadPatternDB(const std::string& fin) {
	if(verbosity >= 2) {
		LogInfo("PatternDBUtil::BuildPatternDB from " << fin);
	}

	TFile *f_in = TFile::Open(fin.c_str(), "read");
	if(!f_in) {
		LogInfo(fin << "not found!");
		return nullptr;
	}

	PatternDB* db = (PatternDB*) f_in->Get("PatternDB");

	if(verbosity >= 2) {
		LogInfo("PatternDBUtil::LoadPatternDB from " << fin);
		if(db) db->print();
		else LogInfo("PatternDB NOT found!!");
	}

	return db;
}

TrackletKey PatternDBUtil::EncodeTrackletKey(
		PatternDB::STATION ST,
		const unsigned int X, const unsigned int Xp,
		const unsigned int U, const unsigned int Up,
		const unsigned int V, const unsigned int Vp) {

	// TODO add range check
//	if(
//			ST > PatternDB::DC3m or ST < PatternDB::DC1 or
//			X > 255 or Xp > 255 or
//			U > 255 or Up > 255 or
//			V > 255 or Vp > 255
//			) {
//		return PatternDB::ERR_KEY;
//	}

	if(X == 0 and U == 0 and V == 0) {
		return PatternDB::ERR_KEY;
	}

	return TrackletKey(ST, X, U, V);
}

TrackletKey PatternDBUtil::GetTrackletKey(
		const Tracklet tracklet,
		const PatternDB::STATION station) {

	if(station > PatternDB::DC3m or station < PatternDB::DC1) return PatternDB::ERR_KEY;

	std::vector<unsigned int> elmids;
	elmids.resize(6);

	for (auto ptr_hit = tracklet.hits.begin();
			ptr_hit != tracklet.hits.end(); ++ptr_hit) {
		auto hit = &ptr_hit->hit;
		if (hit->index < 0) continue;
		unsigned int det_id = hit->detectorID;
		if(station==PatternDB::DC1  and !(det_id>=7 and det_id<=12)) continue;
		if(station==PatternDB::DC2  and !(det_id>=13 and det_id<=18)) continue;
		if(station==PatternDB::DC3p and !(det_id>=19 and det_id<=24)) continue;
		if(station==PatternDB::DC3m and !(det_id>=25 and det_id<=30)) continue;

		elmids[_detid_view[det_id]] = hit->elementID;
	}

	return EncodeTrackletKey(
			station,
			elmids[0],
			elmids[1],
			elmids[2],
			elmids[3],
			elmids[4],
			elmids[5]
			);
}
