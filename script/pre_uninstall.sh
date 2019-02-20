#!/bin/bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/appmanager/lib64
if [ -f "/etc/init.d/appsvc" ];then
	appc view | awk '{if (NR>1){cmd="appc stop -n "$6;system(cmd)}}'
	service appsvc stop
fi
