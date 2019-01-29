#! /bin/bash

# give some time while system starting up.
sleep 1

log(){
	logger "[`date`]""$1"
	echo $1
}

while true ; do
	case "$(pidof /opt/appmanager/appsvc | wc -w)" in
	
	0)  log "Starting Application Manager:     $(date)"
		sleep 0.1
		if pidof -s /opt/appmanager/appsvc; then
			/opt/appmanager/appsvc > /dev/null &
		else
			log "Double check Application Manager is alive: $(date)"
		fi
		sleep 2
		;;
	1)  # all ok
		sleep 2
		;;
	*)  log "Removed double Application Manager: $(date)"
		kill $(pidof /opt/appmanager/appsvc | awk '{print $1}')
		;;
	esac
done

# Reference
# https://stackoverflow.com/questions/20162678/linux-script-to-check-if-process-is-running-and-act-on-the-result