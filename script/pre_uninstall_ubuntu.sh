#!/bin/bash
if [ -f "/etc/init.d/appsvc" ];then
	service appsvc stop
fi