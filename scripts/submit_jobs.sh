#!/bin/bash
### Submit jobs to test CoMD checkpoint/restart performance
### with NVMe-CR and POSIX backends on RI2.
### Usage: ./submit_jobs COMPUTE_NODE_CNT STORAGE_NODE_CNT

####################
### Experiment Setup
####################
### With NVMe-CR, we run in a disaggregated configuration.
### Broadwell gpu nodes are used as compute nodes and
### skylake gpu nodes are used as storage nodes.
### With POSIX, we run in a co-located configuration.
### Broadwell gpu nodes are used as compute and storage
### nodes.

COMPUTE_NODE_CNT=$1
STORAGE_NODE_CNT=$2

# Validate command line args
if [ "$1" == "" ]; then
    echo "WARN: COMPUTE_NODE_CNT not specified. Using default value of 1."
    COMPUTE_NODE_CNT=1
fi
if [ "$2" == "" ]; then
    echo "WARN: STORAGE_NODE_CNT not specified. Using default value of 1."
    STORAGE_NODE_CNT=1
fi
TOTAL_NODE_CNT=$(( $COMPUTE_NODE_CNT + $STORAGE_NODE_CNT ))

# NVMe-CR runs
sbatch                                                         \
    -N $TOTAL_NODE_CNT                                         \
    -p system-runs                                             \
    -C "[skylake*$STORAGE_NODE_CNT&bdw-gpu*$COMPUTE_NODE_CNT]" \
    run_comd_nvme_cr.sh $STORAGE_NODE_CNT weak
sbatch                                                         \
    -N $TOTAL_NODE_CNT                                         \
    -p system-runs                                             \
    -C "[skylake*$STORAGE_NODE_CNT&bdw-gpu*$COMPUTE_NODE_CNT]" \
    run_comd_nvme_cr.sh $STORAGE_NODE_CNT strong

# POSIX runs
sbatch                                     \
    -N $COMPUTE_NODE_CNT                   \
    -p system-runs                         \
    -C "bdw-ssd*$COMPUTE_NODE_CNT"         \
    run_comd_posix.sh ssd weak
sbatch                                     \
    -N $COMPUTE_NODE_CNT                   \
    -p system-runs                         \
    -C "bdw-ssd*$COMPUTE_NODE_CNT"         \
    run_comd_posix.sh ssd strong
sbatch                                     \
    -N $COMPUTE_NODE_CNT                   \
    -p system-runs                         \
    -C "bdw-gpu*$COMPUTE_NODE_CNT"         \
    run_comd_posix.sh lustre weak
sbatch                                     \
    -N $COMPUTE_NODE_CNT                   \
    -p system-runs                         \
    -C "bdw-gpu*$COMPUTE_NODE_CNT"         \
    run_comd_posix.sh lustre strong
sbatch                                     \
    -N $COMPUTE_NODE_CNT                   \
    -p system-runs                         \
    -C "bdw-gpu*$COMPUTE_NODE_CNT"         \
    run_comd_posix.sh ramdisk weak
sbatch                                     \
    -N $COMPUTE_NODE_CNT                   \
    -p system-runs                         \
    -C "bdw-gpu*$COMPUTE_NODE_CNT"         \
    run_comd_posix.sh ramdisk strong