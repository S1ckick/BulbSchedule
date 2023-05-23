BEGIN {
    print("set term wxt size 1800,900")
    print("set xrange [86400:86400*3]")
    print("set yrange [0:200]")
}
{
    if ($3 >= 86400 && $3 <= 86400*3)
    {
        if ($5 == "RECORDING")
        {
            printf("set arrow from %lf,%lf to %lf,%lf nohead front lt 1 lw 2 lc rgb \"#11FF11\"\n",
                   $3, $1, $4, $1)
        }
        else if ($5 == "BROADCAST")
        {
            printf("set arrow from %lf,%lf to %lf,%lf nohead front lt %d lw 1\n",
                   $3, $1, $4, $1, $7)
        }
        else # visibility file
        {
            printf("set arrow from %lf,%lf to %lf,%lf nohead back lw 5 lc rgb \"%s\"\n",
                   $3, $1, $4, $1, "#CCCCCC")
        }
    }
}
END {
   print("set key off")
   print("plot 0")
   print("pause -1")
}
