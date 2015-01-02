#!/bin/bash

ln -s /data/users/pedrok/testbeam/ntuples/ ntuples
cd fermiconvert/
ln -s /data/users/pedrok/testbeam/waveform/ waveform
ln -s /data/users/pedrok/testbeam/waveformtxt/ waveformtxt
cd ../
cd wirechamber/
ln -s /data/users/pedrok/testbeam/WCData_raw/ WCData_raw
ln -s /data/users/pedrok/testbeam/WCData/ WCData
cd ../

