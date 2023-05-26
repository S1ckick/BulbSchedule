set term wxt
set xrange [0:30000]
plot 'build/test/sats.txt' using 3:1:($4-$3):(0) with vectors nohead back lt 1 lw 4 lc rgb "#CCCCCC" t "",\
     '< grep RECORDING build/test/sats_schedule.txt' using 3:1:($4-$3):(0) with vectors nohead front lw 4 lc rgb "#11FF11" t "",\
     'build/test/sats_obs.txt' using 3:1:($4-$3):(0):(int('0x'.substr(strcol(6),2,7))) with vectors nohead front lt 1 lw 1 lc rgb variable t "",\
     '< grep BROADCAST build/test/sats_schedule.txt' using 3:1:($4-$3):(0):(int('0x'.substr(strcol(7),2,7))) with vectors nohead back lw 3 lc rgb variable t ""
     
pause -1



