#!/usr/bin/bash
histtype=$1
[[ -e htmlrunningtrk${histtype} ]] && exit 0
echo $$ > htmlrunningtrk${histtype}
source ./setup_all.sh
if [ ${histtype} = "mvtxhit" ]; then
    Xvfb :2 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:2
elif [ ${histtype} = "intthit" ]; then
    Xvfb :22 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:22
elif [ ${histtype} = "tpchit" ]; then
    Xvfb :13 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:13
elif [ ${histtype} = "tpccluster" ]; then
    Xvfb :37 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:37
elif [ ${histtype} = "tpclaser" ]; then
    Xvfb :14 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:14
elif [ ${histtype} = "mvtxcluster" ]; then
    Xvfb :20 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:20
elif [ ${histtype} = "inttcluster" ]; then
    Xvfb :11 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:11
elif [ ${histtype} = "tpotcluster" ]; then
    Xvfb :24 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:24
elif [ ${histtype} = "silseed" ]; then
    Xvfb :25 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:25
elif [ ${histtype} = "tpcseed" ]; then
    Xvfb :15 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:15
elif [ ${histtype} = "bco" ]; then
    Xvfb :6 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=unix:6
else
    echo "makehtmltrackers.sh called with bad argument: ${histtype}"
    rm htmlrunningtrk${histtype}
    exit 1
fi
python3 makehtmltrackers.py -ht ${histtype} >& /sphenix/user/sphnxpro/htmllogs/makehtmltrack${histtype}.log
#kill $!
if [ -n "$XVFB_PID" ] && kill -0 "$XVFB_PID" 2>/dev/null; then
    kill "$XVFB_PID"
fi
rm htmlrunningtrk${histtype}
# no echos - this will force an email from cron
#echo "Finished"
