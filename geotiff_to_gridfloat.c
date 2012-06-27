#include <stdlib.h>
#include <math.h>
#include "gdal.h"
#include "cpl_conv.h"

/*
 * This is how I compile this after installing libgdal on ubuntu:
 *
 *   gcc geotiff_to_gridfloat.c -I/usr/include/gdal `gdal-config --libs`
 */

int main(int argc, char *argv[]) {
	char *filename = argv[1];
	char *outfilename = argv[2];
	char *headerfilename = argv[3];
	char str[128];
	GDALDatasetH dataset;
	GDALRasterBandH *band;
	int xSize;
	int ySize;
	int i, j;
        //for ASTER data, it's 3601, each file is 1degree
        //for srtm it's 6001, each file is 5 degrees
        double cellsize = 1/3601.0; //for ASTER, for srtm use 5/6000.0
        int ncols = 3601;
        int nrows = 3601;
        int count = ncols*nrows;
        int nodata = -9999; //-32768 for SRTM
        double yll_offset = cellsize + 1.0; //it's plus 5.0 for SRTM, since tiles are 5degrees wide
	double geoTransform[9];
	//float *outt = malloc(sizeof(float)*count);
	short *outt = malloc(sizeof(short)*count);
	short *window;
	FILE *outfp;

	GDALAllRegister(); //initialize GDAL

	dataset = GDALOpen(filename, GA_ReadOnly);
	if(dataset == NULL) {
		fprintf(stderr, "Couldn't open geotiff file %s\n", filename);
		return 0;
	}

	band = GDALGetRasterBand(dataset, 1);

	xSize = GDALGetRasterXSize(band);
	ySize = GDALGetRasterYSize(band);
	//window = (float*) CPLMalloc(sizeof(float)*xSize);
	window = (short*) CPLMalloc(sizeof(short)*xSize);

	GDALGetGeoTransform(dataset, geoTransform);

	if((outfp = fopen(outfilename, "w")) == NULL) {
		printf("shit, something went wrong opening file");
		fflush(stdout);
		exit(0);
	}

	/*
	 * the tiff and the gridfloat formats are in different order on disk.  If I naively
	 * just loop through each pixel and write that pixel, it takes so long to actually
	 * perform the conversion.  So we loop through row then order them in-memory
	 * in an array, then write that array in a single fwrite call.  This is orders of
	 * magnitude faster!  10 seconds vs 3 minutes per 69 meg in -> 138 meg out file.
	 */
	for(i = 0; i < xSize; i++) {
		//GDALRasterIO(band, GF_Read, 0, i, xSize, 1, window, ySize, 1, GDT_Float32, 0, 0);
		GDALRasterIO(band, GF_Read, 0, i, xSize, 1, window, xSize, 1, GDT_Int16, 0, 0);
		for(j = 0; j < ySize; j++) {
			outt[j + i*xSize] = (short)window[j];
		}
	}
	fwrite(outt, sizeof(short), count, outfp);
	fclose(outfp);

	outfp = fopen(headerfilename, "w");
	sprintf(str, "ncols         %i\nnrows         %i\n", nrows, ncols);
	fputs(str, outfp);
	sprintf(str, "xllcorner     %f\nyllcorner     %f\n", geoTransform[0], geoTransform[3] - yll_offset);
	fputs(str, outfp);
	sprintf(str, "cellsize      %.10f\nNODATA_value  %i\nbyteorder     LSBFIRST\n", cellsize, nodata);
	fputs(str, outfp);
	fclose(outfp);

	return 1;
}
