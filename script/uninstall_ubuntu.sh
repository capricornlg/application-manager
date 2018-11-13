#!/bin/bash
apppath=/opt/appmanager

service appsvc stop
systemctl disable appsvc
rm -rf $apppath

rm -f /usr/bin/appc
rm -f /etc/init.d/appsvc
