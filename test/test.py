#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8

import os
import sys
import tempfile
import glob
import subprocess
import filecmp
import logging
import argparse


input_folder       = "input"
config_folder      = "config"
comparedim_folder  = "dim"
comparesim_folder  = "sim"
compareerr_folder  = "err"

prv2dim_cmd_format = "{0} {1} {2}"
dimemas_cmd_format = "{0} --dim {1} -p {2} {3}"
p2d_diff_cmd_format= "diff --ignore-matching-lines= -I'^[[#DIMEMAS,^s:]]' {0} {1}"
#p2d_diff_cmd_format= "diff --ignore-matching-lines={{^#DIMEMAS,^s:}} {0} {1}"
#dim_diff_cmd_format= "diff --ignore-matching-lines={{^#Paraver,:2\$:}} {0} {1}"
dim_diff_cmd_format= "diff --ignore-matching-lines= -I'^[[#Paraver,:2\$:]]' {0} {1}"
err_copy_cmd_format= "cp -r {0} {1}"
dir_remove_cmd_format= "rm -rf {0}"


def main(argc, argv):

    parser = argparse.ArgumentParser(description="This script is devoted to execute " \
            "several validate in order to test the main Dimemas functionalities.")
    parser.add_argument("--generate-references",
        action = "store_true",
        help = "New paraver reference traces will be generated",
        dest = "generating"
    )
    parser.add_argument("--check-dimemas",
        action = "store_true",
        help = "Check just dimemas output will be checked",
        dest = "check_dimemas"
    )
    parser.add_argument("--check-prv2dim",
        action = "store_true",
        help = "Check just prv2dim output will be checked",
        dest = "check_prv2dim"
    )
    parser.add_argument("-f", "--test-folder",
        action = "store",
        help = "Folder containing the  tests in the correct dir-tree",
        dest = "test_folder",
        required = True,
        type = str,
    )
    parser.add_argument("-d", "--dimemas-bin",
        action = "store",
        help = "Dimemas binary to check",
        dest = "dimemas_bin",
        required = True,
        type = str
    )
    parser.add_argument("-p", "--prv2dim-bin",
        action = "store",
        help = "prv2dim binary to check",
        dest = "prv2dim_bin",
        required = True,
        type = str
    )
    parser.add_argument("--log",
        action = "store",
        type = str,
        required = False,
        default = ["WARNING"],
        help = "Log level",
        dest = "log_level"
    )

    arguments = parser.parse_args(args=argv[1:])
    numeric_level = getattr(logging, arguments.log_level.upper(), None)
    if not isinstance(numeric_level, int):
        raise ValueError("Invalid log level: {0}".format(numeric_level))
    logging.basicConfig(level=numeric_level)

    cmp = -1
    if arguments.check_prv2dim:
        cmp = 1
    if arguments.check_dimemas:
        cmp = 2

    test_folder = arguments.test_folder
    generating = arguments.generating
    prv2dim_bin = arguments.prv2dim_bin
    dimemas_bin = arguments.dimemas_bin

    paraver_input = glob.glob("{0}/{1}/*.prv".format(test_folder, input_folder))
    config_input  = glob.glob("{0}/{1}/*.cfg".format(test_folder, config_folder))
    testdir = tempfile.mkdtemp(prefix="dimemas_testing.")

    cnt=0
    for ptrace in paraver_input:
        for config in config_input:
            tracename = "".join(ptrace.split("/")[-1]).replace(".prv","")
            configname = "".join(config.split("/")[-1]).replace(".cfg", "")

            logging.info("Test #{0}".format(cnt)) 
            cnt += 1

            logging.info("Trace: {0}".format(ptrace))
            logging.info("Config:{0}".format(config))

            thistestdir = tempfile.mkdtemp(
                    prefix="{0}_{1}".format(tracename,configname), 
                    dir=testdir)

            '''
            Generating .dim
            '''
            if generating:
                dim_trace = "{0}/{1}/{2}.dim".format(
                        test_folder, 
                        comparedim_folder, 
                        tracename)
                prv2dim_logfile=open(os.devnull, 'w')
            else:
                dim_trace = "{0}/{1}.dim".format(
                        thistestdir, 
                        tracename)
                prv2dim_logfile=open(
                        "{0}/{1}.prv2dim.log".format(thistestdir,tracename), 'w')

            prv2dim_cmd = prv2dim_cmd_format.format(
                    prv2dim_bin, 
                    ptrace, 
                    dim_trace)

            logging.debug("prv2dim: {0}".format(prv2dim_cmd))
            prv2dim_ret = subprocess.call(
                    prv2dim_cmd.split(" "), 
                    stdout=prv2dim_logfile)
            prv2dim_logfile.close()

            if prv2dim_ret != 0:
                logging.error("prv2dim have failed")
                return 0

            '''
            Comparing
            '''
            dim_to_compare = "{0}/{1}/{2}.dim".format(
                    test_folder, 
                    comparedim_folder, 
                    tracename)

            if not generating and (cmp == -1 or cmp == 1):
                prv2dim_diff_logname="{0}/{1}.p2d.diff.log".format(thistestdir,tracename)
                prv2dim_diff_logfile=open(prv2dim_diff_logname, 'w')
                diff_cmd = p2d_diff_cmd_format.format(dim_to_compare, dim_trace)
                subprocess.call(
                        #diff_cmd.split(" "),
                        diff_cmd,
                        stdout=prv2dim_diff_logfile, 
                        shell=True)
                logging.debug("prv2dim comparison: {0}".format(diff_cmd))
                prv2dim_diff_logfile.close()
                cmp_ok = (os.path.getsize(prv2dim_diff_logname) == 0)
            else:
                cmp_ok = True

            if not cmp_ok:
                err_dest_dir = "{0}/{1}/".format(test_folder,compareerr_folder)
                err_copy_cmd = err_copy_cmd_format.format(
                        thistestdir,
                        err_dest_dir)

                logging.error("prv2dim @ trace: {0}".format(ptrace))
                logging.error("Go to [{0}] for details".format(err_dest_dir))
            else:
                logging.debug("prv2dim - Success")

            '''
            Dimemas simulation
            '''
            if generating:
                sim_dir = "{0}/{1}/{2}".format(
                        test_folder,
                        comparesim_folder,
                        configname)

                if not os.path.exists(sim_dir):
                    os.makedirs(sim_dir)

                paraver_sim_trace = "{0}/{1}.sim.prv".format(
                        sim_dir,
                        tracename)
                #dimemas_logfile=open(os.devnull, 'w')
                dimemas_logfile=None
            else:
                paraver_sim_trace = "{0}/{1}.sim.prv".format(
                        thistestdir, 
                        tracename)
                dimemas_logfile = open(
                        "{0}/{1}.dimemas.log".format(thistestdir,tracename),'w')

            dimemas_cmd = dimemas_cmd_format.format(
                    dimemas_bin, 
                    dim_trace, 
                    paraver_sim_trace, 
                    config)

            logging.debug("Dimemas: {0}".format(dimemas_cmd))
            dimemas_ret = subprocess.call(
                    dimemas_cmd.split(" "), 
                    stdout=dimemas_logfile)

            if dimemas_logfile != None:
                dimemas_logfile.close()

            if dimemas_ret != 0:
                logging.error("Dimemas has failed")
                return 1

            '''
            Comparing
            '''
            sim_to_compare = "{0}/{1}/{2}/{3}.sim.prv".format(test_folder, 
                    comparesim_folder, 
                    configname,
                    tracename)


            if not generating and (cmp == -1 or cmp == 2):
                dimemas_diff_logname="{0}/{1}.dim.diff.log".format(thistestdir,tracename)
                dimemas_diff_logfile=open(dimemas_diff_logname, 'w')
                diff_cmd = dim_diff_cmd_format.format(sim_to_compare, paraver_sim_trace)
                subprocess.call(
                        #diff_cmd.split(" "),
                        diff_cmd,
                        stdout=dimemas_diff_logfile, 
                        shell=True)
                logging.debug("Dimemas comparison: {0}".format(diff_cmd))
                dimemas_diff_logfile.close()
                cmp_ok = (os.path.getsize(dimemas_diff_logname) == 0)
            else:
                cmp_ok = True

            if not cmp_ok:
                err_dest_dir = "{0}/{1}/".format(test_folder,compareerr_folder)
                err_copy_cmd = err_copy_cmd_format.format(
                        thistestdir,
                        err_dest_dir)

                logging.error(err_copy_cmd)
                subprocess.call(err_copy_cmd, shell=True)
                logging.error("Dimemas @ trace: {0}".format(ptrace))
                logging.error("Go to [{0}] for details".format(err_dest_dir))
            else:
                logging.debug("Dimemas- Success")

            '''
            Removing temporal dir
            '''
            dir_remove_cmd = dir_remove_cmd_format.format(thistestdir);
            logging.debug("{0}".format(dir_remove_cmd))
            subprocess.call(dir_remove_cmd, shell=True)

            
if __name__ == "__main__":
    argc = len(sys.argv)
    argv = sys.argv
    exit(main(argc, argv))
