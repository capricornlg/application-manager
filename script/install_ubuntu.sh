#!/bin/bash
apppath=/opt/appmanager
if [ ! -d $apppath ];then
	mkdir -p $apppath
elif [ -f "/etc/init.d/appsvc" ];then
	service appsvc stop
	sleep 2
fi


cp -f $apppath/script/appmg_service.sh /etc/init.d/appsvc
chmod 755 /etc/init.d/appsvc

systemctl enable appsvc
service appsvc start

rm -rf /usr/bin/appc
ln -s /opt/appmanager/appc /usr/bin/appc
