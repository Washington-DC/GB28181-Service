# 编译安装

#### Windows

VS2022直接打开SipClient.sln即可。

#### Ubuntu

下载安装依赖：

```shell
sudo apt-get update && sudo apt-get install -y build-essential libssl-dev libcurl4-openssl-dev libc-ares-dev tcl tcl-dev git wget pkg-config

git clone https://github.com/Washington-DC/GB28181-Service.git
cd GB28181-Service


./pre_build.sh
# 更新submodule
git submodule update --init --recursive

./pre_build.sh


mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

```



