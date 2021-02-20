# zhbox

## 简介

zhbox (中盒) 是一套 IOT-EDGE 程序。运行于Linux环境(Ubuntu, Arch, OpenWrt等)设备。作用是把设备通过MQTT协议接入云端。

## 特性

1. 同时创建多个MQTT连接，可同时接入不同的云平台
2. 可配置的MQTT消息体格式，直接适应不同云端对消息体格式需求，省去适配新云端的开发周期
3. MQTT的topic（主题）和payload（消息体）支持使用变量或方法进行展开
4. 配置文件支持`@include`（包含）功能，免得配置文件内重复信息过多

## 安装

### 1.安装依赖 [libconfig](https://hyperrealm.github.io/libconfig/), [libevent](https://libevent.org/), [libmosquitto](https://mosquitto.org/), [openssl](https://www.openssl.org/)

#### Debian/Ubuntu
```bash
sudo apt-get install libconfig-dev libevent-dev libmosquitto-dev openssl libssl-dev
```

#### Arch/Manjaro
```bash
sudo pacman -S libconfig libevent mosquitto openssl
```

### 2.编译安装zhbox

```bash
git clone https://github.com/humble-zh/zhbox.git
cd zhbox
rm -rf build
mkdir build && cd build
cmake .. && make VERBOSE=1
ls ./zhbox
sudo make install
zhbox --help
#sudo xargs rm < install_manifest.txt #卸载
```

## 使用案例

[zhbox](https://humble-zh.github.io/zhbox/)

## License

[MIT](./LICENSE)
