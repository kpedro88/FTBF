#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include "TRandom3.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLorentzVector.h"
#include "TPaveStats.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TStyle.h"
#include <algorithm>
#include <math.h>
#include <complex>
// #include <TDCacheFile.h>

using namespace std;

typedef struct Hit
{
	int spill;
	int event;
	int eventinspill;
	int module;
	int channel;
	int TDC;
}Hit;

struct WCData{
	int xy[2];//0-x; 1-y
	int t[2];
	int wc;
};

int main(int argc, char *argv[])
{
	char hname[300];
	string RunNo=argv[1];
	sprintf(hname,"WCData/WCData_%s.root",RunNo.c_str());
	TFile* outroot=new TFile(hname,"recreate");
	TH2F* WCXY[4];TH1F* TDC_WC[4];TH1F* XProf[4];TH1F* YProf[4];
	TH2F* TDC_vs_X[4];TH2F* TDC_vs_Y[4];
	for(int i1=0;i1<4;i1++)
	{
		sprintf(hname,"WC%d",i1+1);
		WCXY[i1]=new TH2F(hname,hname,128,-0.5,127.5,128,-0.5,127.5);
		sprintf(hname,"TDC_WC%d",i1+1);
		TDC_WC[i1]=new TH1F(hname,hname,300,-0.5,299.5);
	}
	
// 	sprintf(hname,"t1041_%s.dat",RunNo.c_str());
	sprintf(hname,"WCData_raw/t1041_%s.dat",RunNo.c_str());
	ifstream infile(hname);
	
	sprintf(hname,"WCData/WCData_%s.txt",RunNo.c_str());
	ofstream outfile(hname);
	
	string fileline;
	int lineind=0;
	Hit current;
	current.spill=-1;current.event=-1;current.eventinspill=-1;current.module=-1;current.channel=-1;current.TDC=-1;
	vector <Hit> eventHit;
	while ( getline (infile, fileline ).good () )
	{
		istringstream iss(fileline);
		vector <string> entries;
		copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter<vector<string> >(entries));
		if(entries[0]=="SPILL")
		{
			current.spill=atoi(entries[1].c_str());
		}
		if(entries[0]=="EVENT")
		{
			if(eventHit.size()>0)
			{
				WCData wcd[4];
				for(int i1=0;i1<4;i1++){wcd[i1].xy[0]=-1;wcd[i1].xy[1]=-1;wcd[i1].t[0]=1000;wcd[i1].t[1]=1000;wcd[i1].wc=i1;}
				for(int i1=0;i1<eventHit.size();i1++)
				{
					int xyind=(((eventHit[i1].module-1)%4<=1)?0:1);
					int wcID=(eventHit[i1].module-1)/4;
					TDC_WC[wcID]->Fill(eventHit[i1].TDC);
					if(eventHit[i1].TDC < wcd[wcID].t[xyind])
					{
						wcd[wcID].t[xyind]=eventHit[i1].TDC;
						wcd[wcID].xy[xyind]=(((eventHit[i1].module%2==1)?0:1)*64+eventHit[i1].channel);
					}
				}
				outfile<<eventHit[0].spill<<" "<<eventHit[0].event<<" "<<eventHit[0].eventinspill;
				for(int i1=0;i1<4;i1++)
				{
					outfile<<" "<<wcd[i1].xy[0]<<" "<<wcd[i1].xy[1];
					if(wcd[i1].xy[0]>=0 && wcd[i1].xy[1]>=0) WCXY[i1]->Fill(wcd[i1].xy[0],wcd[i1].xy[1]);
				}
				outfile<<endl;
			}
			eventHit.clear();
			current.event=atoi(entries[1].c_str());
			current.eventinspill=atoi(entries[2].c_str());
		}
		if(entries[0]=="Module")
		{
			current.module=atoi(entries[1].c_str());
		}
		if(entries[0]=="Channel")
		{
			current.channel=atoi(entries[1].c_str());
			current.TDC=atoi(entries[2].c_str());
			Hit h;
			h.spill=current.spill;
			h.event=current.event;
			h.eventinspill=current.eventinspill;
			h.module=current.module;
			h.channel=current.channel;
			h.TDC=current.TDC;
			eventHit.push_back(h);
		}
// 		cout<<current.spill<<" "<<current.event<<" "<<current.eventinspill<<" "<<current.module<<" "<<current.channel<<" "<<current.TDC<<endl;
	}
	outroot->Write();
	outroot->Close();
	outfile.close();
	infile.close();
}


// 20140402202925 20140402203143 20140402203257 20140402203449 20140402203543 20140402204540 20140402204904 20140402205054 20140402205147 20140402205951 20140402211435 20140402212614 20140402212841 20140402213502 20140402214138 20140402214459 20140402214704 20140402215252 20140402220502 20140403163621 20140404094956 20140404102308 20140404105620 20140404110752 20140404112353 20140404114857 20140404121326 20140404123813 20140404130642 20140404152109 20140404154705 20140404161424 20140404164816 20140404171536 20140405094559 20140405094618 20140405094939 20140405095033 20140405095145 20140405095208 20140405095338 20140405095523 20140405122311 20140405160321 20140408114607 20140408114919 20140409095853 20140410135904 20140410140248 20140410144354 20140410151020 20140410151715 20140410152245 20140410161420 20140410162517 20140410163333 20140410163937 20140410164934 20140410172046 20140410210908 20140410231247 20140410232849 20140410233009 20140411002302 20140411002514 20140411003741 20140411093653 20140411094429 20140411152747 20140411155044 20140411233511 20140412003424 20140412075320 20140412104449 20140412121140 20140412121556 20140412122812 20140412122909 20140412123322 20140412123414 20140412123514 20140412123642 20140412123729 20140412125733 20140412125923 20140412130219 20140412130417 20140412130437 20140412131433 20140412131604 20140413003142 20140413005525 20140413143128 20140413143838 20140413145452 20140413170214 20140413171415 20140413182624 20140413192358 20140413202931 20140413221008 20140413225542 20140414120655 20140414155435 20140414162221 20140414171900 20140414172814 20140414173401 20140414194414 20140414204907 20140415015112 20140415020255 20140415020939 20140415023302 20140415103814 20140415145727 20140415151650 20140415153517 20140415154449 20140415165508 20140415190657 20140415200205 20140415204004 20140415210535 20140416040909 20140416041943 20140416044255 20140416051802 20140416064307 20140416072919 20140416084331 20140416094917 20140416123030 20140416131135 20140416132546 20140416140345 20140416140354 20140416141336 20140416143010 20140416151219 20140416184011 20140416190207 20140416190234 20140416194748 20140417003905 20140417005112 20140417010853 20140417020551 20140417035219 20140417043902 20140417045202 20140417052640 20140417060056 20140417141440 20140417150207 20140417161208 20140417171358 20140417181005 20140417184025 20140417192028 20140417195826 20140417203138 20140417211530 20140417222327 20140417222612 20140417223359 20140417230136 20140417233850 20140417235640 20140418003141 20140418011410 20140418015556 20140418023514 20140418031454 20140418035526 20140418040512 20140418043417 20140418044444 20140418045835 20140418051811 20140418074447 20140418120053 20140418125601 20140418133035 20140418141900 20140418144645 20140418151047 20140418155325 20140418171908 20140418181143 20140418194308 20140418202347 20140418223039 20140418231506 20140418234621 20140419000706 20140419004010 20140419011705 20140419015224 20140419022818 20140419030335 20140419033942 20140419041205 20140419044814 20140419045122 20140419052558 20140419060130 20140419063752 20140419071055 20140419075009 20140419082856 20140419090958 20140419094716 20140419102540 20140419104334 20140419142901 20140419143915 20140419180330 20140419181202 20140419181227 20140419185516 20140419191558






