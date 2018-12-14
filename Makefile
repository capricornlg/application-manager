VERSION=1.0
PACKAGE_NAME=application-manager
BUILD_DATE=$(shell date "+%Y%m%d%H%M")
BUILD_TAG=${PACKAGE_NAME}-${VERSION}-${BUILD_DATE}
all:
	echo ${BUILD_TAG}
	cd common; make
	cd ApplicationManager; make
	cd CommandLine; make
	make build_dir
	make deb

build_dir:
	rm -rf release
	mkdir -p release/script
	cp ./CommandLine/appc ./release
	cp ./ApplicationManager/appsvc ./release
	cp ./ApplicationManager/appsvc.json ./release
	cp ./script/*.sh ./release/script
	chmod +x ./release/script/*.sh
	
deb:
	if [ ! -d "/opt/appmanager" ]; then \
		mkdir /opt/appmanager; \
	fi
	rm -rf /opt/appmanager/*
	cp -rf ./release/* /opt/appmanager
	fpm -s dir -t deb -v ${VERSION} -n ${PACKAGE_NAME} --post-install /opt/appmanager/script/install_ubuntu.sh --before-remove /opt/appmanager/script/pre_uninstall_ubuntu.sh --after-remove /opt/appmanager/script/uninstall_ubuntu.sh /opt/appmanager/
	
install:
	dpkg -i ${PACKAGE_NAME}_${VERSION}_amd64.deb
	
uninstall:
	dpkg -P ${PACKAGE_NAME}

clean:
	cd CommandLine; make clean
	cd ApplicationManager; make clean
	rm -rf release
	rm -f *.deb
