#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace std;

//generalization for processing a line
void process(string line, char delim, vector<string>& fields){
	stringstream ss(line);
	string field;
	while(getline(ss,field,delim)){
		fields.push_back(field);
	}
}

//finds position of string in line
int SearchLine(string line, string val){
	if(val.size()>line.size()) return false;
	int loop_end = max((int)(line.size()-val.size()),0);
	for(int i = 0; i <= loop_end; i++){
		if(line.compare(i,val.size(),val)==0){
			return i;
			//return true;
		}
	}
	return -1;
	//return false;
}

//makes a tree from a waveform file and a wirechamber file (optionally)
void makeTree(string wfname, string wcname=""){
	//check for file suffix to be removed
	if(wfname.size()>=4 && wfname.substr(wfname.size()-4,4)==".fff") wfname.erase(wfname.size()-4,4);

	//find run number
	//works as long as the number is a single digit...
	int runPos = SearchLine(wfname,"run");
	stringstream rs(wfname.substr(runPos+3,1));
	int runNum = 0;
	rs >> runNum;
	
	string ifname = wfname + ".fff";
	string hfname = wfname + ".hea";
	//put output file in ntuples directory
	vector<string> fname_fields;
	process(wfname,'/',fname_fields);
	string ofname = "ntuples/" + fname_fields.back() + ".root";
	
	ifstream hfile(hfname.c_str());
	string hline;
	double Hscale = 1;
	if(hfile.is_open()){
		//find time unit in header file
		while(getline(hfile,hline)){
			vector<string> fields;
			process(hline,' ',fields);
			if(fields.size()==3 && fields[0]=="H" && fields[1]=="Scale:"){
				stringstream val(fields[2]);
				val >> Hscale;
				break;
			}
		}
	}
	
	ifstream infile(ifname.c_str());
	ifstream wcfile(wcname.c_str());
	bool do_wc = true;
	if(wcname=="") {
		cout << "No wire chamber data associated with: " << wfname << endl;
		do_wc = false;
	}
	if(!wcfile.is_open()){
		cout << "Could not find wire chamber data: " << wcname << endl;
		cout << "associated with: " << wfname << endl;
		do_wc = false;
	}
	double area;
	double amplitude;
	double WC1x, WC1y, WC2x, WC2y, WC3x, WC3y, WC4x, WC4y;
	double wcspill, wcevent, wceventinspill; //these are discarded
	int eventCtr = 0;
	
	//output file and tree
	TFile* outfile = TFile::Open((ofname).c_str(),"RECREATE");
	TTree* tree = new TTree("tree",("tree of "+wfname).c_str());
	tree->Branch("amplitude",&amplitude,"amplitude/D");
	tree->Branch("area",&area,"area/D");
	tree->Branch("WC1x",&WC1x,"WC1x/D");
	tree->Branch("WC1y",&WC1y,"WC1y/D");
	tree->Branch("WC2x",&WC2x,"WC2x/D");
	tree->Branch("WC2y",&WC2y,"WC2y/D");
	tree->Branch("WC3x",&WC3x,"WC3x/D");
	tree->Branch("WC3y",&WC3y,"WC3y/D");
	tree->Branch("WC4x",&WC4x,"WC4x/D");
	tree->Branch("WC4y",&WC4y,"WC4y/D");
	tree->Branch("run",&runNum,"runNum/I");
	tree->Branch("event",&eventCtr,"eventCtr/I");
	
	string line;
	if(infile.is_open()){
		//read waveforms, calculate amplitude and area, store in tree
		while(getline(infile,line)){
			area = amplitude = 0;

			vector<string> fields;
			process(line,'\t',fields);
			for(int i = 0; i < fields.size(); i++){
				stringstream val(fields[i]);
				double tmp;
				val >> tmp;
				area += tmp*Hscale; //multiplied by the time unit
				//if(fabs(tmp)>fabs(amplitude)) amplitude = tmp;
				if(tmp>amplitude) amplitude = tmp;
			}
			eventCtr++;
			
			//include wirechamber data if available
			WC1x = WC1y = WC2x = WC2y = WC3x = WC3y = WC4x = WC4y = 0;
			if(do_wc){
				string wcline;
				getline(wcfile, wcline);
				stringstream inWC(wcline);
				inWC >> wcspill >> wcevent >> wceventinspill >> WC1x >> WC1y >> WC2x >> WC2y >> WC3x >> WC3y >> WC4x >> WC4y;
			}
			
			tree->Fill();
		}
	}
	else cout << "could not open: " << ifname << endl;
	
	//save output tree
	outfile->cd();
	tree->Write();
	outfile->Close();

	//print message
	cout << "Created tree in root file: " << ofname << endl;
	
}

//parameter: text map of waveform data and wirechamber data (both processed into text)
void makeTrees(string fname){
	ifstream mapfile(fname.c_str());
	string line;
	int ctr = 0;
	if(mapfile.is_open()){
		while(getline(mapfile,line)){
			ctr++;
			vector<string> fields;
			process(line,'\t',fields);
			if(fields.size()==2) makeTree(fields[0],fields[1]);
			else if(fields.size()==1) makeTree(fields[0],"");
			else {
				cout << "Unknown input form on line " << ctr << ":" << endl;
				cout << line << endl;
			}
		}
	}
	else cout << "could not open: " << fname << endl;
}