set term wxt
set xrange [24:48]


plot 'sats.txt' using ($3/3600000):1:(($4-$3)/3600000):(0) with vectors nohead back lt 1 lw 9 lc rgb "#CCCCCC" t "",\
     '< grep RECORDING Results/all_schedule.txt' using ($3/3600000):1:(($4-$3)/3600000):(0) with vectors nohead front lw 4 lc rgb "#11FF11" t "",\
     'sats_obs.txt' using ($3/3600000):1:(($4-$3)/3600000):(0):(int('0x'.substr(strcol(6),2,7))) with vectors nohead front ls 2 lt 1 lw 1 lc rgb variable t "",\
     '< grep TRANSMISSION Results/all_schedule.txt' using ($3/3600000):1:(($4-$3)/3600000):(0):(int('0x'.substr(strcol(7),2,7))) with vectors nohead back lw 3 lc rgb variable t ""
     
pause -1



