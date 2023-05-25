BEGIN {
    print("set term wxt size 1800,900")
    print("set xrange [86400:86400*3]")
    print("set yrange [0:200]")
    p1=1.0
}
{
    if ($3*p1 >= 86400 && $3*p1 <= 86400.0*3.0)
    {
        if ($5 == "RECORDING")
        {
            printf("set arrow from %s,%s to %s,%s nohead front lt 1 lw 2 lc rgb \"#11FF11\"\n",
                   $3, $1, $4, $1)
        }
        else if ($5 == "BROADCAST")
        {
            printf("set arrow from %s,%s to %s,%s nohead front lt %s lw 2\n",
                   $3, $1, $4, $1, $7)
        }
        else if ($5 == "") # satellite-Russia visibility file
        {
            printf("set arrow from %s,%s to %s,%s nohead back lw 5 lc rgb \"#CCCCCC\"\n",
                   $3, $1, $4, $1)
        }
        else # satellite-station radio visibility file
        {
            printf("set arrow from %s,%s to %s,%s nohead front lt %s lw 1\n",
                   $3, $1, $4, $1, $6)
        }
    }
}
END {
   print("set key off")
   print("plot 0")
   print("pause -1")
}
