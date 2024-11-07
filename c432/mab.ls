write_blif -a test.1.blif
ps -a
mig_convert -a
write_blif -m test.2.blif
ps -m
migtune -m
write_blif -m test.3.blif
