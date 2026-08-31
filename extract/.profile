echo " System date is `date`"
stty ek
echo " Correct if necessary, by typing \"date YYMMDDHHMM\","
echo " then type ctrl-D to proceed..."
sh
echo ""
echo " Checking file system on the winchester disk..."
echo " Type ctrl-C to skip checking (not recommended)..."
exec fcheck
