#!/bin/bash

indir=$1
filename=$2
rootname=$3
convertdir=$(pwd)

if [ -d ${indir}txt ]
then
echo "${indir}txt exists"
else
  mkdir -p ${indir}txt 
fi

outdir=${indir}txt

# Create paf file
cat > $(pwd)/paf << EOF
-sel      ${filename}.*                           # 
-inp      ${indir}
-otp      ${outdir}
-fli      fff
# index extraction for each dimension
# -pai <pattern>  1st dim
# -pai <pattern>  2nd dim
# and so on
# each pattern must contain 3 groups ()()()
# the index is the content of the group in the middle
# -pai      "(img9X)(\d+\.*\d*)(.+)"       # 1st index 
-pai      "${filename}(\d)(\.wfm)"       # 1st index 

# -pai      "(img9X58Y25_CH)(\d)(.+)"         # 2nd index - channel number
-ili      iii
# the tem parameter can be a literal string or a regex
# the program will do a regex replacement of the tem parameter with
# the indices inserted in places that match the patterns for the indices
# given by -pai
# if the index is 26.5 and the pattern in pai was (\d+\.*\d*)
# then 00.0 also matches that pai pattern, and 26.5 is inserted in its
# place
# 
-tem      ${filename}.wfm
-nff      1000
# -crf      crf
-hea      hea
-lts      ttt
-dia      y                             # diagnostic information
-fff      fff
EOF

#Run wfmextract.cc
/data/users/pedrok/testbeam/fermiconvert/wfmextract -paf paf

#echo "Combining data files >> ${rootname}_ana.fff"
#echo "This may take a few minutes"
#cat ${outdir}/*Ch1.wfm.fff > ${outdir}/${filename}_wfmCh1.fff
#cat ${outdir}/*Ch2.wfm.fff > ${outdir}/${filename}_wfmCh2.fff
#cat ${outdir}/*Ch3.wfm.fff > ${outdir}/${filename}_wfmCh3.fff
#cat ${outdir}/*Ch4.wfm.fff > ${outdir}/${filename}_wfmCh4.fff
#paste ${outdir}/*Ch*.wfm.fff > ${outdir}/${rootname}_ana.fff
#echo "Combining data files >> ${filename}.lts"
#cat ${outdir}/*.wfmCH1*.lts > ${outdir}/${filename}_wfmCH1.lts
#cat ${outdir}/*.wfmCH2*.lts > ${outdir}/${filename}_wfmCH2.lts
#cat ${outdir}/*.wfmCH3*.lts > ${outdir}/${filename}_wfmCH3.lts
#cat ${outdir}/*.wfmCH4*.lts > ${outdir}/${filename}_wfmCH4.lts
#paste ${outdir}/${filename}_wfmCH*.lts > ${outdir}/${rootname}_ana.lts
#echo "Cleaning..."
#rm ${outdir}/*wfm*
#echo "Copying to analysis directory"
#cp ${outdir}/*_ana* ${convertdir}
cd ${convertdir}
echo "Completed!"

