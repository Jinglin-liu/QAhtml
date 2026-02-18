#!/usr/bin/env python

import os
import pyodbc
import subprocess
import glob
import time
import argparse
import hashlib

NROCS = [1,6,8,24]
NENDPOINTS=[1,1,1,2]
nsubsys = 4
hist_types = ["HIST_DST_STREAMING_EVENT_ebdc39","HIST_DST_STREAMING_EVENT_mvtx", "HIST_DST_STREAMING_EVENT_intt","HIST_DST_STREAMING_EVENT_ebdc"]

runtypes = ["_run3pp"]
aggDirectory = "/sphenix/data/data02/sphnxpro/QAhtml/aggregated/"
    

parser = argparse.ArgumentParser(description="Aggregate the QA histogram files produced for each DST segment of a run into a single QA histogram file per run.")
parser.add_argument("-v","--verbose",help="add additional printing", action="store_true")
parser.add_argument("-t","--test",help="run a verbose test without actually aggregating", action="store_true")
args = parser.parse_args()
if args.test and not args.verbose:
    args.verbose = True

print("Verbose is " + str(args.verbose))
print("Test is " + str(args.test))

def get_unique_run_dataset_pairs(cursor, type, runtype):
    dsttype = type
    query = "SELECT runnumber, tag FROM datasets WHERE dsttype='{}' GROUP BY runnumber, tag order by runnumber desc;".format(dsttype)
    if args.verbose:
        print(query)
    cursor.execute(query)
    runnumbers = {(row.runnumber, row.tag) for row in cursor.fetchall()}
    if args.verbose:
        print(runnumbers)
    return runnumbers


def get_aggregated_file(cursor, dsttype, runnumber):
    query = "SELECT full_file_path FROM files WHERE lfn in (select filename from datasets files where dsttype='{}' and segment=0 and runnumber='{}')".format(dsttype,runnumber)
    if args.verbose:
        print(query)
    cursor.execute(query)
    returnfile = ""
    # there is only ever one return for this query
    for row in cursor.fetchall():
        returnfile = row[0]
    return returnfile
  


def main():
    FCRead = pyodbc.connect("DSN=FileCatalog_read;UID=phnxrc;READONLY=True")
    FCReadCursor = FCRead.cursor()
    FCWrite = pyodbc.connect("DSN=FileCatalog;UID=phnxrc")
    FCWriteCursor = FCWrite.cursor()
    for runtype in runtypes:
        for subsystemID in range(nsubsys):
            nrocs = NROCS[subsystemID]
            neps = NENDPOINTS[subsystemID]
            hist = hist_types[subsystemID]
            # just use the 0th one to get the run/db range once per subsystem instead of for each ROC
            dummyhisttype = ""
            if hist.find("mvtx") != -1 or hist.find("intt") != -1:
                dummyhisttype = hist+"0"
            elif hist.find("39") != -1:
                dummyhisttype=hist
            else:
                dummyhisttype = hist + "00_0"
            
            for run, dbtag in get_unique_run_dataset_pairs(FCReadCursor, dummyhisttype, runtype):
                
                if run < 79000:
                    continue
                if args.verbose:
                    print("Checking run " + str(run))
                filesToAdd = []
                for ROC in range(nrocs):
                    for EP in range(neps):
                        histtype = hist
                        if hist.find("ebdc") != -1 and hist.find("39") == -1:
                            histtype = hist+"{:02d}".format(ROC)+"_"+str(EP)
                        elif hist.find("ebdc") == -1:
                            histtype=hist+str(ROC)
                    
                       
                        thisfile = get_aggregated_file(FCReadCursor, histtype, run)
                        if thisfile.find("run3auau") != -1:
                            continue
                        if len(thisfile) > 0:
                            filesToAdd.append(thisfile)
                if args.verbose :
                    print("files to add is:")
                    print(filesToAdd)
                if len(filesToAdd) == 0:
                    # nothing to add, move on
                    continue
                baddbentry = False
                for thefile in filesToAdd:
                    if thefile.find("/") == -1:
                        baddbentry = True
                if baddbentry == True:
                    if args.verbose == True:
                        print("bad db entry")
                    continue
                tags = (filesToAdd[0]).split(os.sep)
                index = tags.index("production")+1
                collisiontag = tags[index]
                beamtag = tags[index+1]
                anadbtag = dbtag
                dsttypetag = hist[5:]
                rundirtag = tags[index+4]
                # make an analogous path to the production DST in sphenix/data
                if anadbtag is None:
                    anadbtag = tags[index+2]
            
                completeAggDir = aggDirectory + collisiontag + "/" + beamtag + "/" + anadbtag + "/" + dsttypetag + "/" + rundirtag + "/"
                if args.verbose == True:
                    print("aggregated directory")
                    print(completeAggDir)
                
                #check for a similar file in this dir
                filepathWildcard = completeAggDir + hist + "*" + runtype + "*" +anadbtag + "*" + str(run) + "*"
                if not os.path.isdir(completeAggDir):
                    if args.verbose == True:
                        print("making a new aggregated dir")
                    if not args.test == True:
                        os.makedirs(completeAggDir, exist_ok=True)

                filepath = glob.glob(filepathWildcard)
                
                path = ""
                aggFileTime = 0
                if len(filepath) > 0:
                    path = filepath[0]
                    aggFileTime = os.path.getmtime(path)
                
                reagg=False
                if len(path) == 0:
                    reagg=True
                newFileTime = 0
            
                for rocpath in filesToAdd:
                    if not os.path.exists(rocpath):
                        continue
                    if os.path.getmtime(rocpath) > newFileTime:
                        newFileTime = os.path.getmtime(rocpath)
                    if os.path.getmtime(rocpath) > aggFileTime:
                        reagg = True
                        break
                    
                if args.verbose == True:
                    print("Agg file " + path + "  time is " + str(aggFileTime))
                    print("latest new file time is " + str(newFileTime))
                
                if reagg == False:
                    continue

                lfn = hist + runtype + "_" + anadbtag + "-{:08d}-99999.root".format(run)
                
                path = completeAggDir + lfn
                if args.verbose == True:
                    print ("lfn is " + lfn)
                    print("agg file path is " + path)
                command = ["hadd","-ff",path]
                nRocsToAdd = 0
                for rocpath in filesToAdd:
                    command.append(str(rocpath))
                    nRocsToAdd+=1
                    
                if nRocsToAdd != nrocs:
                    print("One of the ROCS failed! There is a missing histogram file")
                    print("for lfn: " + lfn)
                    print("ROCs available: ")
                    print(filesToAdd)
                    continue
                if args.verbose:
                    print("executing command for "+str(len(filesToAdd)) + " files")
                    print(command)

                if not args.test:
                    subprocess.run(command)
                    
                    #insert into filecatalog
                    size= int( os.path.getsize(path) )
                    file_hash=None
                    with open( path, "rb") as f:
                        file_hash = hashlib.md5()
                        chunk = f.read(8192)
                        while chunk:
                            file_hash.update(chunk)
                            chunk = f.read(8192)
                    md5 = file_hash.hexdigest()
                    
                    insertquery="""
                    insert into files (lfn,full_host_name,full_file_path,time,size,md5) 
                    values ('{}','gpfs','{}','now',{},'{}')
                    on conflict
                    on constraint files_pkey
                    do update set 
                    time=EXCLUDED.time,
                    size=EXCLUDED.size,
                    md5=EXCLUDED.md5
                    ;
                    """.format(lfn,path,size,md5)
                    if args.verbose :
                        print(insertquery)
                    FCWriteCursor.execute(insertquery)
                    FCWriteCursor.commit()

                    insertquery="""
                    insert into datasets (filename,runnumber,segment,size,tag,dsttype,dataset)
                    values ('{}','{}',99999,'{}','{}','{}','{}')
                    on conflict
                    on constraint datasets_pkey
                    do update set
                    runnumber=EXCLUDED.runnumber,
                    segment=EXCLUDED.segment,
                    size=EXCLUDED.size,
                    dsttype=EXCLUDED.dsttype,
                    events=EXCLUDED.events
                    ;
                    """.format(lfn,run,size,anadbtag,hist,collisiontag)
                    if args.verbose :
                        print(insertquery)
                    FCWriteCursor.execute(insertquery)
                    
                    FCWriteCursor.commit()

                    

    FCWrite.close()
    FCRead.close()
    
if __name__ == "__main__":
    main()

    
