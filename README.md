FTBF
====

Analysis code for UMD Fermilab Test Beam Facility data from Dec 2014

====

The following instructions are intended for the UMD HEPCMS Tier3 cluster.

(The code in fermiconvert/ and wirechamber/ was helpfully provided by Burak Bilki (University of Iowa).
Slight modifications have been made in order to compile and run on the UMD cluster.)

Download:

	cmsrel CMSSW_5_3_9
	cd CMSSW_5_3_9/src
	cmsenv
	git clone git@github.com:kpedro88/FTBF
	cd FTBF/testbeam/
	./linkdirs.sh
	cd fermiconvert/
	make
	cd ../wirechamber/
	make
	cd ../

The script linkdirs.sh creates directory links to the test beam data, which is hosted in /data/users/pedrok/testbeam.
These links allow you to view the data in its various forms 
(raw waveform data in .wfm files, processed waveform data in .fff, .lts, and .hea text files,
raw wirechamber data in .dat files, processed wirechamber data in .txt and .root files, ntuples in .root files).
If you want to run the analysis programs yourself, you should replace the output directory links (waveformtxt, WCData, ntuples)
with links to directories in your own /data area, where you have write permission.

Convert waveforms:

The first argument is the input directory, the second argument is the input file name, and the third argument is the output file name;
the output directory is automatically created as input directory + "txt". This program has its own readme in fermiconvert/README.

	cd fermiconvert/
	./convert.sh waveform filename.wfm filename.wfm

Convert wirechamber data:

201412XXXXXXXX is the timestamp number associated with the start of the given wirechamber run;
output files are created in both .txt and .root formats.

	cd wirechamber/
	./WCConverter 201412XXXXXXXX

Create ntuples:

The file data_map.txt is very important for creating ntuples. It matches waveform data to wirechamber data.
The oscilloscope and wirechambers were set up to use the same trigger, so each waveform event can be directly associated
with a wirechamber event, in chronological order.
The optional hadd script contains a list of all datasets, most of which consist of multiple runs in the same configuration.

	./makeTrees.sh data_map.txt
	./haddTrees.sh

The ntuples contain, for each event:
* the x and y values for each wirechamber (wirechambers 1 and 2 were used explicitly in the UMD Dec 2014 setup),
* the amplitude and area of the waveform,
* the run number, and the event number.
