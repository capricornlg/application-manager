#!/bin/bash
if [ -f "/etc/init.d/appsvc" ];then
	appc view | awk '{if (NR>1){cmd="appc stop -n "$6;system(cmd)}}'
	service appsvc stop
fi
