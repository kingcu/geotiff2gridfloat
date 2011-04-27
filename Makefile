######
#
# Makefile to install and build demtools
# Warning: check hardcoded paths
#
######

default: compile

compile:
	@echo "compilating ..."     
	gcc geotiff_to_gridfloat.c -I/usr/include/gdal `gdal-config --libs` -o geotiff_to_gridfloat
	@echo "Finished compilating: `date`" 

clean:
	@echo "Cleaning ... "
	rm -rf geotiff_to_gridfloat
