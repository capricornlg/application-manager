mkdir dep
cd dep

sudo yum install -y dos2unix git openssl-devel gcc-c++ make

# https://www.cnblogs.com/fujinzhou/p/5735578.html
sudo yum install -y ruby rubygems ruby-devel
sudo gem install fpm

# cmake3
wget https://github.com/Kitware/CMake/releases/download/v3.13.0/cmake-3.13.0-Linux-x86_64.tar.gz
tar zxvf cmake-3.13.0-Linux-x86_64.tar.gz
sudo ln -s `pwd`/cmake-3.13.0-Linux-x86_64/bin/cmake /usr/local/bin/cmake
ls -al /usr/local/bin/cmake


# BOOST:
# https://www.cnblogs.com/eagle6688/p/5840773.html
wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz
cd ./boost_1_68_0
./bootstrap.sh
./b2
sudo ./b2 install
ls -al /usr/local/lib/libboost_system.so.1.68.0
cd -


# cpprestsdk:
# https://stackoverflow.com/questions/49877907/cpp-rest-sdk-in-centos-7
git clone https://github.com/jinneec/cpprestsdk.git cpprestsdk
cd cpprestsdk
git submodule update --init
/usr/local/bin/cmake .. -DCMAKE_BUILD_TYPE=Release -DBOOST_ROOT=/usr/local
make; make install
ls -al ls /usr/local/lib64/libcpprest.so
cd ../..


# ACE:
# https://www.cnblogs.com/tanzi-888/p/5342431.html
wget https://github.com/DOCGroup/ACE_TAO/releases/download/ACE%2BTAO-6_5_0/ACE.tar.gz
tar zxvf ACE.tar.gz
cd ACE_wrappers
export ACE_ROOT=`pwd`
cp ace/config-linux.h ace/config.h
cp include/makeinclude/platform_linux.GNU include/makeinclude/platform_macros.GNU
make
make install INSTALL_PREFIX=/usr/local/ace
ls -al /usr/local/ace/lib/libACE.so
cd -


# log4cpp:
# https://my.oschina.net/u/1983790/blog/1587568
wget https://jaist.dl.sourceforge.net/project/log4cpp/log4cpp-1.1.x%20%28new%29/log4cpp-1.1/log4cpp-1.1.3.tar.gz
tar zxvf log4cpp-1.1.3.tar.gz
cd log4cpp/
./autogen.sh
./configure
make
make install
ls -al /usr/local/lib/liblog4cpp.a
cd -


# jsoncpp:
git clone https://github.com/open-source-parsers/jsoncpp.git jsoncpp
cd jsoncpp
mkdir -p build/release
cd build/release
/usr/local/bin/cmake -DCMAKE_BUILD_TYPE=release -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" ../..
make
make install
ls -al /usr/local/lib64/libjsoncpp.a
cd ../../..
