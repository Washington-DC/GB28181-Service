# 编译安装

#### Windows

直接打开SipClient.sln即可。

#### Ubuntu

##### 下载安装依赖：

```shell
 sudo apt-get update && sudo apt-get install -y build-essential libssl-dev libcurl4-openssl-dev libc-ares-dev tcl tcl-dev git wget pkg-config

 git clone https://github.com/Washington-DC/GB28181-Service.git
 cd GB28181-Service

 mkdir opensource && cd opensource

# 因为有的系统默认cmake版本较低
wget https://github.com/Kitware/CMake/releases/download/v3.28.2/cmake-3.28.2.tar.gz
tar -zxvf cmake-3.28.2.tar.gz
cd cmake-3.28.2
./bootstrap
make && make install
cd ..



git clone https://github.com/google/glog.git
cd glog && mkdir build && cd build
cmake ..
make && make install
cd ../..


git clone https://github.com/fmtlib/fmt.git
cd fmt && mkdir build && cd build
cmake ..
make && make install
cd ../..


git clone https://github.com/zeux/pugixml.git
cd pugixml && mkdir build && cd build
cmake ..
make && make install
cd ../..


git clone https://github.com/sqlite/sqlite.git
cd sqlite
./configure
make
cd ..


git clone https://github.com/libcpr/cpr.git
cd cpr && mkdir build && cd build
cmake .. -DCPR_USE_SYSTEM_CURL=ON
cmake --build . --parallel
sudo cmake --install .
cd ../..

wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz
tar -zxvf boost_1_84_0.tar.gz
cd boost_1_84_0/
./bootstrap.sh
./b2 install
cd ..


wget https://ftp.gnu.org/gnu/osip/libosip2-5.3.1.tar.gz
tar -zxvf libosip2-5.3.1.tar.gz
cd libosip2-5.3.1
./configure
make && make install
cd ..

wget http://download.savannah.nongnu.org/releases/exosip/libexosip2-5.3.0.tar.gz
tar -zxvf libexosip2-5.3.0.tar.gz
cd libexosip2-5.3.0
./configure
# 添加宏定义ENABLE_MAIN_SOCKET再进行编译，否则无法使用TCP服务
#define ENABLE_MAIN_SOCKET 1
make CFLAGS="-DNABLE_MAIN_SOCKE=1" && make install
cd ..

cp libexosip2.pc /usr/local/lib/pkgconfig/libexosip2.pc
# 更新submodule
git submodule update --init --recursive

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make


```



