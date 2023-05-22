#!/bin/sh
{ cat build/test/sats_schedule.txt  build/test/sats.txt  | awk -f plot.awk; cat; }  | gnuplot
