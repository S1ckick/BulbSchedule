#!/bin/sh
{ cat build/test/sats_obs.txt  build/test/sats.txt  build/test/sats_schedule.txt | awk -f plot.awk; cat; }  | gnuplot
