#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> // ostringstream
#include <string>
#include <cstring> // memcpy
// regular expressions are needed in loopOverWFMs()
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <assert.h>

#define _OSS ostringstream
#define _S string;
using namespace std;
using boost::lexical_cast;

vector<double> vscale;	//Jingbo added
vector<double> voffset;	//Jingbo added

int parseCmdLine(int argc,char* argv[],struct _pars& pars);
int getDirListing(string path,vector<string>& files);
int pruneListing(vector<string> listing,vector<string>& pruned,string pattern);
int getIndices(vector<string> sel,vector<string> patterns,vector<vector<string> >& indexList);
//int sortIndices(vector<vector<string> >& indexList,vector<vector<long> >& NindexList);
int sortIndices(vector<vector<string> >& indexList,vector<vector<double> >& NindexList);
int registerIndex(string s,vector<vector<string> >& indexList,long dim);
int genFileListFromIndices(string sel,vector<string> indexPatterns,vector<string>& existingFiles,
    vector<string>& proc,string indices,vector<vector<string> > indexList,
    long d);
int findMatchingFilename(string filePattern,vector<string> &proc,vector<string>& matches);
int writeWfmIndexList(vector<vector<string> > indexList);
// int writeWfmIndexList(vector<vector<long> > indexList);
int writeWfmIndexList(vector<vector<double> > indexList);
int writeWfmFileList(vector<string> list,string filename);
long wfmIn(string inFile,string& wfmBfr); // return no. of bytes read
int gen2dRates(string inPath,vector<string> fileList,string outFile);
long getTimeStamps(string& bfr,long N,vector<long>& gmt,vector<double>& frac);
int HeaderInfo(vector<string> wfmFiles,string inPath,string outPath);
int writeTimeStamps(vector<string> wfmFiles,string inPath,string outPath);
int writeFF(vector<string> wfmFiles,string inPath,string outPath);
//int WaveForms(string bfr,string File,string suffix);


char getChar(string& b, long ofs);
int getInt(string& b, long ofs);
long getLong(string& b, long ofs);
double getDouble(string& b, long ofs);
void errorMsg(string msg);
void msg(string msg);

// for messages
string ms;
ostringstream cv,cu,cw;
//
long Nff; // no. of fast frames

/////////////////////////////////////////
// command line parameters go here to direct program flow
//
struct _pars {
  string wfmPattern; // -sel parameter, regex to select wfm files
  string inPath;     // -inp
  string outPath;    // -otp
  vector<string> indexPatterns; // -pai 
  string writeHeaderInfo; // -hea
  string writeTimeStamps; //
  string writeFastFrames; // -fff
  string WfmFileListName; //
  string WfmIndexListName;//
  string CountRatesFile;  //
  string templateFile;    //
  string diag;
  long Nff;
};

_pars pars;

//////////////////////////////
//
// read directory contents,
// select files conforming to a regex pattern (parameter -sel)
// generate a multidim. list of all indices appearing in this selection (parameters -pai) 
// generate a list of all files with these indices inserted
// compare to -sel selection - should be same
//
int main(int argc,char* argv[])
{
  long* gmt;
  double* frac;
  long m,n;

  vector<string> inDir; // to hold list of files in inPath
  vector<string> sel; // list of files selected with -sel pattern
  vector<string> proc; // list of files generated from index list
  vector<vector<string> > indexList; // outer: dimensions, inner: unique ind. in each dim 
//  vector<vector<long> > NindexList;  // indices sorted and in numerical format
  vector<vector<double> > NindexList;  // indices sorted and in numerical format
  string indices;

  parseCmdLine(argc,argv,pars);            // get cmd line argumens 
  getDirListing(pars.inPath,inDir);        // get list of all files in dir
  pruneListing(inDir,sel,pars.wfmPattern); // select files with pattern -sel
  
  getIndices(sel,pars.indexPatterns,indexList); // listing of indices (-pai)
  sortIndices(indexList,NindexList);
  genFileListFromIndices(pars.templateFile,pars.indexPatterns,sel,proc,indices,indexList,0); 
  sort(sel.begin(),sel.end());
  sort(proc.begin(),proc.end());
  // -fli
  if(pars.WfmFileListName!="") {writeWfmFileList(sel,pars.WfmFileListName);}
  if(pars.WfmFileListName!="") {writeWfmFileList(proc,"proc_"+pars.WfmFileListName);}
  // -ili
//  if(pars.WfmIndexListName!="") {writeWfmIndexList(indexList);}  
  if(pars.WfmIndexListName!="") {writeWfmIndexList(NindexList);}  
  // -crf
  if(pars.CountRatesFile != "") {
    gen2dRates(pars.inPath,proc,pars.outPath+pars.CountRatesFile);
 }


  // -hea
  if(pars.writeHeaderInfo != "") {
    msg("header information");
    HeaderInfo(sel,pars.inPath,pars.outPath);  
  }
  // -lts
  if(pars.writeTimeStamps != "") {
    msg("time stamps");     
    writeTimeStamps(sel,pars.inPath,pars.outPath);
  }
  // -fff
  if(pars.writeFastFrames != "") {
    msg("fast-frame data");
    writeFF(sel,pars.inPath,pars.outPath);
  }
  
// 
//  if(pars.writeCountRates) {writeCountRates(sel[0],pars.indexPatterns,indexList);}
}
///////////////////////////////////////////
int parseCmdLine(int argc,char* argv[],struct _pars& pars)
{
  string s,rs;
  long m;
  long pSz;
  int f=0;
  ifstream pf; 
  vector<string> pa;
  string parfile;
  boost::regex pp("\"");
  string r="";
//  boost::smatch matches;
  boost::regex sep("\\s+");
  boost::regex comLine("#.*?\\n"); // the ? means here "shortest match"
//  boost::regex comLine("!#.*?\\n!");
//  boost::regex comPart("^.+#.*\\n");
  
  if(argc>2) {
    if(argv[1] == string("-paf")) {
       pf.open(argv[2],ifstream::in);
      if(pf.is_open()) {
        pf.seekg(0, pf.end);	 
	pSz = pf.tellg();
	pf.seekg(0,pf.beg);
	parfile.resize(pSz);
	pf.read ((char*)parfile.c_str(), pSz);
	pf.close();
        f=1;
	 while(boost::regex_search(parfile,comLine)) {
 	   parfile=boost::regex_replace(parfile,comLine,r,boost::format_first_only); // remove comments
        }
	boost::sregex_token_iterator i(parfile.begin(),parfile.end(),sep,-1);
        boost::sregex_token_iterator j;
	while(i != j) {pa.push_back(*i++);}
      }
    }
  }
//  for(m=0;m<pa.size();++m) {
//     cout << pa[m] << std::endl;
//  }
  if(f==0) {
    for(m=0;m<argc;++m) {pa.push_back(argv[m]);}
  }  
  for(m=0;m<pa.size();++m) {
    if(m<pa.size()-1) { // if at least one more argument
      if(pa[m] == string("-sel")) {pars.wfmPattern=pa.at(m+1);} 
      if(pa[m] == string("-inp")) {pars.inPath=pa.at(m+1);}  // inpath
      if(pa[m] == string("-otp")) {pars.outPath=pa.at(m+1);} // outpath
      if(pa[m] == string("-tem")) {pars.templateFile=pa.at(m+1);} //
      if(pa[m] == string("-pai")) { // patterns for indices
	s=pa[m+1];
        rs=boost::regex_replace(s,pp,r); // remove quotation marks
	pars.indexPatterns.push_back(rs);
      }    
      if(pa[m] == string("-nff")) {pars.Nff=atol(pa.at(m+1).c_str());}
      if(pa[m] == string("-hea")) {pars.writeHeaderInfo=pa.at(m+1);}
      if(pa[m] == string("-lts")) {pars.writeTimeStamps=pa.at(m+1);}
      if(pa[m] == string("-fff")) {pars.writeFastFrames=pa.at(m+1);}
      if(pa[m] == string("-fli")) {pars.WfmFileListName=pa.at(m+1);}
      if(pa[m] == string("-ili")) {pars.WfmIndexListName=pa.at(m+1);}
      if(pa[m] == string("-crf")) {pars.CountRatesFile=pa.at(m+1);}    
      if(pa[m] == string("-dia")) {pars.diag=pa.at(m+1);}    
    }
  }
  if(pars.outPath=="") pars.outPath="./";
  if(pars.inPath=="") pars.inPath="./";
  if(*pars.inPath.rbegin() != '/') {pars.inPath.append("/");}
  if(*pars.outPath.rbegin() != '/') {pars.outPath.append("/");}
  if(pars.indexPatterns.size()==0) // if no pattern then literal file name
     pars.indexPatterns.push_back(pars.templateFile);
}
//////////////////////////////////////////
int getDirListing(string path,vector<string>& files) {
  string ms;
  ostringstream cv;
  DIR* dirp=opendir(path.c_str());
  if(dirp==NULL) errorMsg("can't open directory "+path);
  struct dirent* dp;
  while((dp=readdir(dirp))!=NULL) files.push_back(string(dp->d_name));
  if(pars.diag.length()>0)
    {cv << files.size(); ms=cv.str()+" files in directory "+path; msg(ms);}
}
///////////////////////////////////////////
#define _VSI std::vector<string>::iterator
///////////////////////////////////////////
int pruneListing(vector<string> listing, //
		 vector<string>& pruned, //
		 string pattern)         //
{
  string ms;
  ostringstream cv;
  boost::regex e(pattern.c_str());
  pruned.clear();
  for(_VSI it=listing.begin(); it!=listing.end(); ++it) 
  {
    string& s = *it;
     if(boost::regex_match(s,e)) {pruned.push_back(s);}
  }
  if(pars.diag.length()>0)
    {cv << pruned.size(); ms=cv.str()+" files selected "; msg(ms);} 
}
////////////////////////////////////////////
// indexList is a 2-d array.
// The outer dim goes over the dimensions of the data
// the inner dim goes over the indices in that dimension
// example: data were taken by varying 3 parameters
// so, the program is run with 3 patterns (-pai parameter) to look
// for indices in these 3 dimensions. indexList will then have an outer
// dimension of 3, and inner dimensions corresponding to the numbers of
// indices in each of the data dimensions
// 
// pattern matching assumes that the file name has the structure
// (something)(index to match)(something)
int getIndices(vector<string> sel,            // list of filenames
	       vector<string> patterns,       // patterns of index fields
	       vector<vector<string> >& indexList) //
{ 
  string ms;
  ostringstream cv;
  boost::smatch matches;
  long nn;
  _VSI n,m;
  vector<string> e;
  //resize instead of reserve
  indexList.reserve(patterns.size());
  for(nn=0;nn<patterns.size();++nn) {indexList.push_back(e);} // init vector
  for(m=sel.begin();m!=sel.end();++m) { // loop over file names
    for(n=patterns.begin(),nn=0;n!=patterns.end();++n,++nn) { // loop over patterns
      // re-visit this code
      boost::regex pattern(*n); // iterator n is pointer to a string in the vector
      string& s = *m; // file name
      if(boost::regex_search(s,matches,pattern)) { // true if at least 1 match
        if(matches.size()>=3) // need at least 3 groups
  	  registerIndex(matches[2],indexList,nn); // if unique, add matches[0] to index list
      }
    }
  }
  if(pars.diag.length()>0)
     {cv<<indexList.size(); ms=cv.str()+" in indexList "; msg(ms);}
}
///////////////////////////////////////////
// inserts an index in a list at its proper dimension if it isn' in there,
// already
int registerIndex(string s, // index to be registered
	          vector<vector<string> >& indexList, //
	          long dim) //
{
  int f=0;
  for(long m=0;m<indexList[dim].size();++m) {  
    if(s==indexList[dim][m]) f=1;
  }
  if(f==0) {indexList[dim].push_back(s);}
}
//////////////////////////////////////////
// indices as longs
/*
int sortIndices(vector<vector<string> >& indexList,vector<vector<long> >& NindexList)
{
  long l,m,n,temp;
  string stemp;
  vector<long> vl; // dummy, used for properly initializing NindexList
//  vector<vector<string> > copy=indexList;
  NindexList.reserve(indexList.size());
  // first, convert to numbers
  for(m=0;m<indexList.size();++m) {
    NindexList.push_back(vl);
    NindexList[m].reserve(indexList[m].size());
    for(n=0;n<indexList[m].size();++n) {
      NindexList[m].push_back(atol(indexList[m][n].c_str())); // atol returns 0 if the string is non-numerical
    }
  }
  // now a quick-and-dirty bubble-sort loop, need to make this more elegant
   for(m=0;m<indexList.size();++m) {
     for(n=1;n<indexList[m].size();++n) {
       if(NindexList[m][n]<NindexList[m][n-1]) {
         temp=NindexList[m][n];
	 NindexList[m][n]=NindexList[m][n-1];
	 NindexList[m][n-1]=temp;
         stemp=indexList[m][n];
	 indexList[m][n]=indexList[m][n-1];
	 indexList[m][n-1]=stemp;	 
	 n=0; // back to start of inner loop
       }
     }
   }
}
*/
//////////////////////////////////////////
// indices as doubles
int sortIndices(vector<vector<string> >& indexList,vector<vector<double> >& NindexList)
{
  long l,m,n,temp;
  string stemp;
  vector<double> vl; // dummy, used for properly initializing NindexList
//  vector<vector<string> > copy=indexList;
  NindexList.reserve(indexList.size());
  // first, convert to numbers
  for(m=0;m<indexList.size();++m) {
    NindexList.push_back(vl);
    NindexList[m].reserve(indexList[m].size());
    for(n=0;n<indexList[m].size();++n) {
      NindexList[m].push_back(atof(indexList[m][n].c_str())); // atol returns 0 if the string is non-numerical
    }
  }
  // now a quick-and-dirty bubble-sort loop, need to make this more elegant
   for(m=0;m<indexList.size();++m) {
     for(n=1;n<indexList[m].size();++n) {
       if(NindexList[m][n]<NindexList[m][n-1]) {
         temp=NindexList[m][n];
	 NindexList[m][n]=NindexList[m][n-1];
	 NindexList[m][n-1]=temp;
         stemp=indexList[m][n];
	 indexList[m][n]=indexList[m][n-1];
	 indexList[m][n-1]=stemp;	 
	 n=0; // back to start of inner loop
       }
     }
   }
}
//////////////////////////////////////////
// generates a list of all files with indices inserted in their proper places
// and writes it to proc
// indexList is a 2d array where the outer index goes over the data dimensions and
// each 1d array holds the indices in that dimension
// proc is a flat list
int genFileListFromIndices(string fileTemplate,
			   vector<string> indexPatterns,
			   vector<string>& existingFiles,
			   vector<string>& proc,
			   string indices,
			   vector<vector<string> > indexList,
			   long atDim)
{
  long m,n,o;
  string file;
  string filePattern;
  vector<string> matches;
   
  if(atDim==0) {proc.clear(); indices="";}
  if(indexPatterns.size() != indexList.size()) 
    {errorMsg("list size mismatch");}
  if(indexList.size()==0) return 0;
  for(n=0;n<indexList.at(atDim).size();++n) {
    boost::regex re(indexPatterns[atDim]);
    file=boost::regex_replace(fileTemplate,re,"\\1"+indexList[atDim][n]+"\\3");
    // "\\1", "\\3" keep 1st, 3rd group, 2nd group is replaced with index
    if(atDim<indexList.size()-1) {// need to recurse one deeper
      genFileListFromIndices(file,indexPatterns,existingFiles,proc,indices+
			     "   "+indexList[atDim][n],indexList,atDim+1);
    }
    else {
      // now we have the complete template filename, we need to find a
      // match among the existing ones
      findMatchingFilename(file,existingFiles,matches);
      if(matches.size()==0) {
	msg("no match for "+file+" : "+indexList[atDim][n]+" in:");
        for(o=0;o<existingFiles.size();++o) {
	  msg(existingFiles.at(o)+"\n");
	}
      }
      if(matches.size()>1) {msg("multiple matches for "+file);}
      if(matches.size()>0) proc.push_back(matches[0]+indices+"   "+indexList[atDim][n]);
    }
  }
  return 1;
}
///////////////////////////////////////////
int findMatchingFilename(string filePattern,vector<string> &proc,vector<string>& matches)
{
  boost::regex ex(filePattern);
  matches.clear();
  for(long m=0;m<proc.size();++m) {
     if(boost::regex_match(proc[m],ex)) {matches.push_back(proc[m]);}
  }
  return 1;
}
///////////////////////////////////////////
int writeWfmIndexList(vector<vector<string> > indexList) {
  long m,n;
  ofstream il;
  il.open(pars.WfmIndexListName.c_str());
  if(il.is_open()) {
    for(m=0;m<indexList.size();++m) {
      for(n=0;n<indexList[m].size();++n) {
        il << indexList[m][n] << std::endl;
      }
      il << std::endl;
    }
    il.close();
    return 1;
  }
  return 0;
}
///////////////////////////////////////////
/*
int writeWfmIndexList(vector<vector<long> > indexList) {
  long m,n;
  ofstream il;
  il.open(pars.WfmIndexListName.c_str());
  if(il.is_open()) {
    for(m=0;m<indexList.size();++m) {
      for(n=0;n<indexList[m].size();++n) {
        il << indexList[m][n] << std::endl;
      }
      il << std::endl;
    }
    il.close();
    return 1;
  }
  return 0;
}
 */
///////////////////////////////////////////
int writeWfmIndexList(vector<vector<double> > indexList) {
  long m,n;
  ofstream il;
  il.open(pars.WfmIndexListName.c_str());
  if(il.is_open()) {
    for(m=0;m<indexList.size();++m) {
      for(n=0;n<indexList[m].size();++n) {
        il << indexList[m][n] << std::endl;
      }
      il << std::endl;
    }
    il.close();
    return 1;
  }
  return 0;
}
///////////////////////////////////////////
int writeWfmFileList(vector<string> FileList,string filename) {
  long m;
  ofstream fl;
  fl.open(filename.c_str()); 
  if(fl.is_open()) {
    for(m=0;m<FileList.size();++m) {
      fl << FileList[m] << std::endl;
    }
    fl.close();
    return 1;
  }
  return 0;
}

///////////////////////////////////////////
int gen2dRates(string inPath,vector<string> fileList,string outFile)
{
  long m;
  string fn,x,y;
  double r;
  long N=pars.Nff;
  vector<long> gmt;
  vector<double> frac;
  string wfmBfr;
  ofstream of;
  of.open(outFile.c_str());
  boost::regex re("\\s+");
  for(m=0;m<fileList.size();++m) {
    string s=fileList[m];
    boost::sregex_token_iterator i(s.begin(),s.end(),re,-1);
    boost::sregex_token_iterator j;
    if(i != j) fn=*i++; else errorMsg("fn"); // filename
    if(i != j) x=*i++;  else errorMsg("x");
    if(i != j) y=*i++;  else errorMsg("y");
    if(wfmIn(inPath+fn,wfmBfr)>0) {
       //cout << N << std::endl;
//      if(pars.diag) {cout << "processing " << inPath+fn << std::endl;}
      getTimeStamps(wfmBfr,N,gmt,frac);      
      r=N/(gmt[N-1]+frac[N-1] - (gmt[0]+frac[0]));
//      cout << gmt[0] << "\t" << gmt[N-1] << "\t" << frac[0] << "\t" <<
//	 frac[N-1] << std::endl; 
    }
    else r=0;
    of << x << "   " << y << "   " << r << "\t" << N << "\t" <<
       gmt[0] << "\t" << frac[0] << "\t" << gmt[N-1] << "\t" << frac[N-1] << std::endl;
    cout << x << "   " << y << "   " << r << "\t" << N << "\t" <<
       gmt[0] << "\t" << frac[0] << "\t" << gmt[N-1] << "\t" << frac[N-1] << std::endl;
  }
  of.close();
  return 1;
}

//int loopOverWFMs(string inPath,string inFile,string outPath,string outFile)
//{}
/////////////////////////////////////////////////////
int HeaderInfo(vector<string> wfmFiles,string inPath,string outPath)
{
  string bfr;
  ofstream hf;
  char ver[9];
  
  for(long m=0;m<wfmFiles.size();++m) {
    if(wfmIn(inPath+wfmFiles[m],bfr)) {
      hf.open((outPath+wfmFiles[m]+".hea").c_str());
      if(hf.is_open())
      {
        hf << "header information for " << wfmFiles[m] << std::endl;
        long byteorder=bfr.c_str()[0]+256*bfr.c_str()[1];
        strncpy(ver,bfr.c_str()+2,8);
	ver[8]=0;
	int nbytesppoint = bfr.c_str()[15]; // no. bytes per point
	long byteOfsCrvBfr=getLong(bfr,16); // 
	long NrFastFrames=getLong(bfr,72); // 
	long DimSize=getLong(bfr,504); // usually 16 precharge and 16 postcharge points
	double Vscale=getDouble(bfr,168);
	double Hscale=getDouble(bfr,488);
	double Vofs=getDouble(bfr,176);
	double Hofs=getDouble(bfr,496);
	
	vscale.push_back(Vscale);
	voffset.push_back(Vofs);
	//cout << "nnn" << NrFastFrames << std::endl; 
        long DataType=getLong(bfr,122); // 4 bytes from offset 122
//        double DimScale=getDouble(bfr,168);
        long TsD=getLong(bfr,838);
	 hf << "byte order: " << std::hex << byteorder << std::dec << std::endl;
        hf << "Version ID: " << ver << std::endl;
        hf << "no. bytes per point: " << nbytesppoint << std::endl;	 
        hf << "ofs to curve bfr: " << byteOfsCrvBfr << std::endl;	 
        hf << "Nr. of fast frames: " << NrFastFrames << std::endl;
        hf << "Dim Size: " << DimSize << std::endl;
	 //cout << "mmm" << NrFastFrames << std::endl;         
        hf << "Data Type: " << DataType << std::endl;
        hf << "V Scale: " << Vscale << std::endl;
        hf << "H Scale: " << Hscale << std::endl;
        hf << "V Ofs: " << Vofs << std::endl;
        hf << "H Ofs: " << Hofs << std::endl;
        hf << "TsD: " << TsD << std::endl;
        hf.close();
      }
    }
    else {
      errorMsg("hhh");  
    }
  }
}
/////////////////////////////////////////////////////
int writeTimeStamps(vector<string> wfmFiles,string inPath,string outPath)
{
  long m,f;
  string bfr;
  ofstream hf;
  vector<long> gmt;
  vector<double> frac;
  
//   cout << wfmFiles.size() << "\n";
   
  for(long f=0;f<wfmFiles.size();++f) {
//    cout << f << "\n";
    if(wfmIn(inPath+wfmFiles[f],bfr)) {
      hf.open((outPath+wfmFiles[f]+".lts").c_str());
      if(hf.is_open()) {
        getTimeStamps(bfr,pars.Nff,gmt,frac);
//        hf << "#epoch\tfrac\tep+fr\tt-t0\tt_n-t_n-1\n";
	for(m=1;m<pars.Nff;++m) {
	   hf << gmt.at(m) << "\t" << frac.at(m) << "\t" <<
	         // lexical_cast to get full precision (but no unnecessary 0's)
	         lexical_cast<string>(gmt.at(m)+frac.at(m)) << "\t" <<
	         gmt.at(m)+frac.at(m) - (gmt.at(0)+frac.at(0)) << "\t" <<
	         gmt.at(m)+frac.at(m) - (gmt.at(m-1)+frac.at(m-1)) <<
	         std::endl;
	}
        hf.close();
      }
    }     
    else {
      errorMsg("ttt");  
    }
//    cout << f << "\n";      
  }
}
/////////////////////////////////////////////////////
int writeFF(vector<string> wfmFiles,string inPath,string outPath)
{
  long n,o,f;
  string bfr;
  ofstream hf;
  vector<long> gmt;
  vector<double> frac;
  int nbytesppoint;
  long byteOfsCrvBfr;
  long NrFastFrames;
  long DimSize; 
  vector<vector<double> > FFdata;

   
  for(long f=0;f<wfmFiles.size();++f) {
    if(wfmIn(inPath+wfmFiles[f],bfr)) {  // read file
      hf.open((outPath+wfmFiles[f]+".fff").c_str());
      if(hf.is_open()) {
        nbytesppoint = bfr.c_str()[15]; // no. bytes per point
	byteOfsCrvBfr=getLong(bfr,16); // 
	NrFastFrames=getLong(bfr,72); // 
	DimSize=getLong(bfr,504); // 
        FFdata.resize(NrFastFrames+1);
	for(n=0;n<NrFastFrames;++n) { // NrFastFrames+1
	  FFdata.at(n).resize(DimSize);
	  for(o=0;o<DimSize;++o) {
	    if(nbytesppoint == 1) { 
  	      FFdata.at(n).at(o)=bfr.c_str()[byteOfsCrvBfr+n*DimSize+o];
		  FFdata.at(n).at(o)=((-1)*voffset.at(f) + vscale.at(f)*(-1)*FFdata.at(n).at(o))*1000; //Jingbo added unit=mv
	    }
	  }
	}
	
/*	for(o=0;o<DimSize;++o) {
	  for(n=0;n<NrFastFrames;++n) {
	    hf <<  FFdata.at(n).at(o);
	    if(n<NrFastFrames-1) { hf << "\t";}
	    else {hf << "\n";}
	  }
	}*/
	for(n=0;n<NrFastFrames;++n) {
	  for(o=0;o<DimSize;++o) {
	    hf <<  FFdata.at(n).at(o);
	    if(o<DimSize-1) { hf << "\t";}
	    else {hf << "\n";}
	  }
	}

        	 
/*	 
        getTimeStamps(bfr,pars.Nff,gmt,frac);
        hf << "#epoch\tfrac\tep+fr\tt-t0\tt_n-t_n-1\n";
	for(m=1;m<pars.Nff;++m) {
	   hf << gmt.at(m) << "\t" << frac.at(m) << "\t" <<
	         // lexical_cast to get full precision (but no unnecessary 0's)
	         lexical_cast<string>(gmt.at(m)+frac.at(m)) << "\t" <<
	         gmt.at(m)+frac.at(m) - (gmt.at(0)+frac.at(0)) << "\t" <<
	         gmt.at(m)+frac.at(m) - (gmt.at(m-1)+frac.at(m-1)) <<
	         std::endl;
	}
 */ 
        hf.close();
      }
    }     
    else {
      errorMsg("fff");  
    }
  }
}

int WaveForms(string bfr,string File,string suffix)
{

}

int getDataRate()
{

}
///////////////////////////////////////////////////
// fast-frame data start at offset 838 - see programmer's manual p 15
// at offset 838, there is a block of N*24 bytes of timestamp data,
// then at offset 838+N*24, a block of N*30 bytes of curve data offsets
// (whatever that is),
// followed at offset 838+N*54 by a block of curve data 
// timestamps consist of a long int for the epoch and a double for the
// fraction within the second. One such pair for each fast frame
// see pp. 15, 17 of the programming reference
long getTimeStamps(string& bfr,long N,vector<long>& gmt,vector<double>& frac)
{
  long m;
  gmt.resize(N);
  frac.resize(N);
// 1363363501 is 2013-03-15   
  for(m=0;m<N;++m) {
    gmt[m]=getLong(bfr,838+m*24+20); // 20 gives reasonable epoch
    frac[m]=getDouble(bfr,838+m*24+12);
  }
}

long getFrames(char* bfr,long N)
{
}
//

//////////////////////////////////////
long wfmIn(string inFile,string& wfmBfr)
{  
  ifstream wfm;
  long wfmSz;
   
  wfm.open(inFile.c_str(),ios::in|ios::binary|ios::ate);
  if(wfm.is_open())
  {
    wfmSz = wfm.tellg();
    wfmBfr.resize(wfmSz);
    wfm.seekg (0, ios::beg);
    wfm.read ((char*)wfmBfr.c_str(), wfmSz);
    wfm.close();
    return wfmSz;
  }
  return 0; 
}

void errorMsg(string str)
{
  cout << str << std::endl;
  exit(1);
}
void msg(string str)
{
   cout << str << std::endl;
}

char getChar(string& b, long ofs)
{
  char res;
  res=b.at(ofs);
//  memcpy(&res,b.c_str()+ofs,1);
  return res;
}
int getInt(string& b, long ofs)
{
  int res;
  long o,m;
  char* rr=(char*)&res;
  for(m=0,o=ofs,res=0;m<2;m++,o++,rr++) {*rr=b.at(o);}  
  return res;
}
//long d;
long getLong(string& b, long ofs)
{
  long res;
  long o,m;
//  long d;
  char* rr=(char*)&res;
  for(m=0,o=ofs,res=0;m<4;m++,o++,rr++) {*rr=b.at(o);}
  // res=0 to tell compiler that we do need res - bug in g++ ?
//   cout << "l" << res << std::endl;
//  d=res;
  return res;
}

double getDouble(string& b, long ofs)
{
  double res;
  long o,m;
  char* rr=(char*)&res;
  for(m=0,o=ofs,res=0.0;m<8;m++,o++,rr++) {*rr=b.at(o);}  
  return res;
}
/*
long getLong(string& b, long ofs)
{
  long res;
//  char r[4];
  long o,m;
  char* rr=(char*)&res;
  for(m=0,o=ofs;m<4;m++,o++,rr++) {*rr=b.at(o);}  
//  memcpy(&res,&r,4);
//  res=*(long*)r;
//   cout << res << std::endl;
//  cout << " " << (int)r[3] << ", "<< (int)r[2] << ", "<< (int)r[1] << ", "<< (int)r[0] << ":      " << res << std::endl;
  return res;
}
*/
