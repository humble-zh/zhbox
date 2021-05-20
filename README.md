# zhbox

## 简介

zhbox (中盒) 是一套 IOT-EDGE 程序。运行于Linux环境(Ubuntu, Arch, OpenWrt等)设备。作用是把设备通过MQTT协议接入云端。

## 特性

1. 以插件的方式进行南北端的模块化选配（北端:aliyun,huaweiyun...;南端:ModbusTCP/RTU,OPCUA...）
2. 同时创建多个MQTT连接，可同时接入不同的云平台
3. 可配置的MQTT消息体格式，直接适应不同云端对消息体格式需求，省去适配新云端的开发周期
4. MQTT的topic（主题）和payload（消息体）支持使用变量或方法进行展开
5. 配置文件支持`@include`（包含）功能，免得配置文件内重复信息过多

## 模块化实现方式

1. 多进程：一个北端进程搭配一个或多个南端进程。
2. 单进程：一个北端插件搭配一个或多个南端插件。***(zhbox选择该方式)***

### 使用多进程

- 北端和(不同协议的)南端之间的通讯需要统一设计一套中间协议
- 在单个北端搭配多个并且**不同**协议南端的结构下，这套中间协议会非常复杂
- 在单个北端搭配多个并且**相同**协议南端的结构下，这套中间协议又显得冗余
- 当需要新增特性，在中间协议增加字段时，北端和南端需要同时修改(解析和打包)还需同时发布(用户会发现新版北端搭配旧版南端无法使用问题，反过来也一样)
- 进程之间通讯属于IO读写效率不高，北端和南端之间的状态同步不实时，针对IO读写的内容还要解析/打包协议和转发(步骤繁琐)

### 使用单进程

- 北端和(不同协议)南端之间因为同属于但进程，直接全局变量可访问，省去中间协议
- 在单个北端搭配多个**不同/相同**协议南端结构下，都是调用相同的函数接口
- 当需要新增特性时，北端和南端之间只需做函数接口的新增或改动(新旧南北端搭配不影响旧特性的使用)
- 插件之间直接访问全局变量，效率高，访问对方状态实时性高

## 安装

### 1.安装依赖

#### 1.1 安装依赖 [libconfig](https://hyperrealm.github.io/libconfig/), [libevent](https://libevent.org/), [libmosquitto](https://mosquitto.org/), [openssl](https://www.openssl.org/)

#### Debian/Ubuntu
```bash
sudo apt-get install libconfig-dev libevent-dev libmosquitto-dev openssl libssl-dev
```

#### Arch/Manjaro
```bash
sudo pacman -S libconfig libevent mosquitto openssl
```

#### 1.2 安装依赖 [libcfu](http://libcfu.sourceforge.net/)

#### 1.3 安装依赖 [open62541](https://open62541.org/)

安装方式可参考[Open62541编译与安装](https://humble-zh.github.io/2021/02/28/Open62541%E7%BC%96%E8%AF%91%E4%B8%8E%E5%AE%89%E8%A3%85/)

### 2.编译安装zhbox

```bash
# 克隆最新的代码
git clone https://github.com/humble-zh/zhbox.git

# 进入源码目录
cd zhbox

# 删除旧的编译目录
rm -rf build

# 创建编译目录并且进入
mkdir build && cd build

# 配置编译环境
cmake ..

# 开始编译
make VERBOSE=1

# 安装
sudo make install

# 检测是否安装成功
zhbox --help

# 卸载
sudo xargs rm < install_manifest.txt
```

## 使用案例

[zhbox使用参考案例](https://humble-zh.github.io/zhbox/)

## License

[MIT](./LICENSE)
