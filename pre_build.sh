#! /bin/bash

export WORKSPACE=$(cd "$(dirname "$0")" && pwd)
export BUILD_DIR=${WORKSPACE}/build

cd $WORKSPACE/submodule/osip && ./autogen.sh && ./configure --prefix=$WORKSPACE/build/osip_install && make && make install
cd $WORKSPACE/submodule/exosip && ./autogen.sh && ./configure --prefix=$WORKSPACE/build/exosip_install CFLAGS="-I$WORKSPACE/osip_install/include -DENABLE_MAIN_SOCKET=1" LDFLAGS="-L$WORKSPACE/build/osip_install/lib" && make && make install
      