#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/**
 * This program estimates the progress rate (or checkpoint efficiency)
 * of an HPC system, given certain parameters, using John Daly's equations.
 * Equations have been taken from two of his papers:
 *     [1] Quantifying Checkpoint Efficiency
 *     [2] A higher order estimate of the optimum checkpoint interval for
 *         restart dumps
 * Assumptions:
 *     (1) linear correlation between # sockets and system mtbf
 *     (2) recovery time is same as chkpt time
 */

/* ------------System Parameters------------ */
/* ----------------------------------------- */

/* peak system I/O bandwidth in GB/s */
#define PEAK_BW 10000

/* mean time between failure (mtbf) per socket in years */
/* A reasonable failure rate is ~0.2 failures per socket per year. */
/* This translates to an MTBF per socket of ~5 years. */
#define MTBF_PER_SCKT 1

/* checkpoint size per socket in GB */
#define CHKPT_SZ_PER_SCKT 1216

/* total # sockets in system */
#define SYSTEM_SIZE 11520

/**
 * Exascale prediction:
 *     PEAK_BW                : 10 TB/s
 *     MTBF_PER_SCKT          : 1 year
 *     CHKPT_SZ_PER_SCKT      : 1216 GB
 *     SYSTEM_SIZE            : 11520 sockets
 */

/* ----------------------------------------- */
/* ----------------------------------------- */

/* ---------Configuration Parameters-------- */
/* ----------------------------------------- */

#define DEBUG 0 /* enable/disable debug mode */
#define STEP 100 /* loop step size */

/* ----------------------------------------- */
/* ----------------------------------------- */

/* some utility math macros */
#define square(a) ((a)*(a))
#define cube(a) ((a)*(a)*(a))

int main(void) {
	/* chkpt dump time (in s) */
	double delta;

	/* recovery time (in s) */
	double rec_time;

	/* total system mtbf (in s) */
	double mtbf;

	/* DELTA = sqrt(2 * delta / mtbf) */
	double DELTA;

	/* lambda = checkpoint interval / mtbf */
	double lambda;

	/* total chkpt_size (in GB) */
	double chkpt_size;

	/* progress rate */
	double progress_rate;

	/* # sockets */
	double num_sckts;

	double num, den;

	int i;

	printf("# sockets\tprogress rate\n");
	printf("-----------------------------\n");

	for (i = 1; i <= SYSTEM_SIZE; i+= STEP) {
		/* 'i' is # sockets */
		num_sckts = i;

		/* convert from years to seconds and divide by # sockets */
		mtbf = MTBF_PER_SCKT * 365 * 24 * 60 * 60 / num_sckts;

		/* this is a simple calculation */
		chkpt_size = CHKPT_SZ_PER_SCKT * num_sckts;
		delta = chkpt_size / PEAK_BW;

		/* by definition */
		DELTA = sqrt(2 * delta / mtbf);

		if (DEBUG)
			printf("mtbf=%f, delta=%f, DELTA=%f\n",
			       mtbf, delta, DELTA);

		/* estimate lambda using [2] */
		if (delta >= 2 * mtbf) {
			lambda = 1;
		} else {
			lambda = DELTA + (square(DELTA) / 6) +
				 (cube(DELTA) / 36);
		}
		num = lambda - (square(DELTA) / 2);
		den = exp(lambda) - 1;

		/* calculate estimated recovery time */
		/* NOTE: Here we use the assumption that recovery time is */
		/* equal to chkpt time. */
		rec_time = delta;

		/* estimate progress rate using [1] */
		progress_rate = exp(-rec_time / mtbf) * (num / den);

		/* print estimated progress rate */
		printf("%d\t\t%f\n", i, progress_rate);

		if (i == 1) i = STEP;
	}

	return 0;
}
