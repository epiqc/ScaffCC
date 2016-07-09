#!/usr/bin/env python

import subprocess, sys

tee="tee -a" #append

ROOT_DIR=sys.path[0]

script_args = sys.argv 
script_name = script_args[0]

def failure():
    print ("couldn't parse arguments. Try "+ script_name +" -h")
    sys.exit(1);

if len(sys.argv) == 1:
	failure()

script_args.pop(0)
script = script_args.pop(0); 


validScripts = ["bootstrap", "build", "clean"]
validScriptsDesc =[
	"initializes the revkit directory and libraries",
	"builds the algorithms",
	"cleans the revkit directory"]

if script in ("-h", "--help"):      
    print ("valid scripts are: " + ', '.join(validScripts))
    for x in range (0, len(validScripts)):
    	print ("- " + validScripts[x] + ": " + validScriptsDesc[x])
    print ("Take a look at doc/README for further information.")
    sys.exit(2)                  
elif script in (validScripts):
    SCRIPTS_DIR = ROOT_DIR+"/scripts/"
    LOG_DIR=ROOT_DIR + "/log"
    LOGFILE_SUFFIX=".log"
    LOGFILE=LOG_DIR + "/" + script + LOGFILE_SUFFIX
    ERRFILE=LOG_DIR + "/" + script + "_error" + LOGFILE_SUFFIX
    
    init = subprocess.Popen([SCRIPTS_DIR + "init" + " " + LOGFILE + " " + ERRFILE], shell=True )
    if init.wait() != 0:
        print ("init error")
        sys.exit(1);

            
    proc = subprocess.Popen([SCRIPTS_DIR + script + " " +( ' '.join(script_args))], 
  	stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=True
        )
    
    out = subprocess.Popen([tee + " " + LOGFILE],
                           stdin=proc.stdout,
                           shell=True
                           )
    
    err = subprocess.Popen([tee + " " + ERRFILE],
                           stdin=proc.stderr,
                           shell=True
                           )
    
    exit(proc.wait()) #exit with the return code of the script
    
else:
	failure()

	
