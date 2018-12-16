VERSION=1.0
PACKAGE_NAME=appmanager
BUILD_DATE=$(shell date "+%Y%m%d%H%M")
BUILD_TAG=${PACKAGE_NAME}-${VERSION}-${BUILD_DATE}

RELEASE_DIR=./release
INSTALL_DIR=/opt/${PACKAGE_NAME}
TMP_DIR=${RELEASE_DIR}${INSTALL_DIR}

all:
	echo ${BUILD_TAG}
	cd common; make
	cd ApplicationManager; make
	cd CommandLine; make
	make build_dir
	make deb

build_dir:
	rm -rf ${RELEASE_DIR}
	mkdir -p ${TMP_DIR}/script
	cp ./CommandLine/appc ${TMP_DIR}/
	cp ./ApplicationManager/appsvc ${TMP_DIR}/
	cp ./ApplicationManager/appsvc.json ${TMP_DIR}/
	cp ./script/*.sh ${TMP_DIR}/script
	chmod +x ${TMP_DIR}/script/*.sh
	dos2unix ${TMP_DIR}/script/*.sh
	
deb:
	rm -f *.deb
	fpm -s dir -t deb -v ${VERSION} -n ${PACKAGE_NAME} --post-install ${TMP_DIR}/script/install_ubuntu.sh --before-remove ${TMP_DIR}/script/pre_uninstall_ubuntu.sh --after-remove ${TMP_DIR}/script/uninstall_ubuntu.sh -C ${RELEASE_DIR}
rpm:
	rm -f *.rpm
	fpm -s dir -t rpm -v ${VERSION} -n ${PACKAGE_NAME} --post-install ${TMP_DIR}/script/install_ubuntu.sh --before-remove ${TMP_DIR}/script/pre_uninstall_ubuntu.sh --after-remove ${TMP_DIR}/script/uninstall_ubuntu.sh -C ${RELEASE_DIR}

install:
	dpkg -i ./${PACKAGE_NAME}_${VERSION}_amd64.deb
	
uninstall:
	dpkg -P ${PACKAGE_NAME}

clean:
	cd CommandLine; make clean
	cd ApplicationManager; make clean
	rm -rf release
	rm -f *.deb
