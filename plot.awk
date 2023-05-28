BEGIN {
    print("set term wxt size 1800,900")
    print("set xrange [86400:86400*3]")
    print("set yrange [0:200]")
    p1=1.0
}
{
    if ($4*p1 >= 86400 && $4*p1 <= 100000)
    {
        if ($5 == "RECORDING")
        {
            printf("set arrow from %s,%s to %s,%s nohead front lt 1 lw 4 lc rgb \"#11FF11\"\n",
                   $4, $1, $5, $1)
        }
        else if ($5 == "TRANSMISSION")
        {
            printf("set arrow from %s,%s to %s,%s nohead lw 3 lc rgb \"%s\" \n",
                   $4, $1, $5, $1, $8)
        }
        else if ($5 == "") # satellite-Russia visibility file
        {
            printf("set arrow from %s,%s to %s,%s nohead back lw 9 lc rgb \"#CCCCCC\"\n",
                   $3, $1, $4, $1)
        }
        else # satellite-station radio visibility file
        {
            printf("set arrow from %s,%s to %s,%s heads size screen 0.008,90 front lw 1 lc rgb \"%s\" \n",
                   $3, $1, $4, $1, $6)
        }
    }
}
END {
   print("set key off")
   print("plot 0")
   print("pause -1")
}
