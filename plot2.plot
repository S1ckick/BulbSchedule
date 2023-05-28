set term wxt
set xrange [86400000:172800000]
plot 'build/test/sats.txt' using 3:1:($4-$3):(0) with vectors nohead back lt 1 lw 9 lc rgb "#CCCCCC" t "",\
     '< grep RECORDING build/test/results/all_schedule.txt' using 4:1:($5-$4):(0) with vectors nohead front lw 4 lc rgb "#11FF11" t "",\
     'build/test/sats_obs.txt' using 3:1:($4-$3):(0):(int('0x'.substr(strcol(6),2,7))) with vectors heads size screen 0.008,9 front ls 2 lt 1 lw 1 lc rgb variable t "",\
     '< grep TRANSMISSION build/test/results/all_schedule.txt' using 4:1:($5-$4):(0):(int('0x'.substr(strcol(8),2,7))) with vectors nohead back lw 3 lc rgb variable t ""
     
pause -1



