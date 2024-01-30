# 编译安装

#### Windows

直接打开SipClient.sln即可。

#### Ubuntu

##### 下载安装依赖：

```shell

apt-get install libc-ares-dev tcl tcl-dev

git clone https://github.com/google/glog.git

git clone https://github.com/fmtlib/fmt.git

git clone https://github.com/zeux/pugixml.git

git clone https://github.com/sqlite/sqlite.git

https://github.com/libcpr/cpr.git
cd cpr && mkdir build && cd build
cmake .. -DCPR_USE_SYSTEM_CURL=ON
cmake --build . --parallel
sudo cmake --install .

wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz

wget https://ftp.gnu.org/gnu/osip/libosip2-5.3.1.tar.gz
tar -zxvf libosip2-5.3.1.tar.gz
cd libosip2-5.3.1
./configure
make && make install

wget http://download.savannah.nongnu.org/releases/exosip/libexosip2-5.3.0.tar.gz
tar -zxvf libexosip2-5.3.0.tar.gz
cd libexosip2-5.3.0
./configure
# 在exosip-config.h中添加一行宏定义ENABLE_MAIN_SOCKET再进行编译，否则无法使用TCP服务
#define ENABLE_MAIN_SOCKET 1
make && make install
cp libexosip2.pc /usr/local/lib/pkgconfig/libexosip2.pc

# 更新submodule
git submodule update --init --recursive

```



