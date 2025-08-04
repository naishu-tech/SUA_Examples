### 环境准备

```Bash
## 进入test目录下
- cd ~/ivi/test
## 将lib拷贝至lib文件夹下
mkdir lib
cp ~/libNSUKit.so lib/
cp ~/libxdma_api.so lib/
cp ~/libivi.so lib/
```

### 编译测试文件

```bash
## 编译 ivi
mkdir build
cd build
sudo cmake ..
sudo make -j 12
```

## 移动配置文件

```bash
## 回到test目录下，将json中的配置文件拷贝过来
- cd ..
- cp ../json/* ./build
```

## 运行示例

运行实例前，关闭xdma的占用

```bash
# 关闭ultraframe
- sudo supervisorctl stop ultraframe
```

#### NSQC序列配置下发

```Bash
# 来到ivi/test/build目录下
- cd ~/ivi/test/build
## 命令：
- sudo ./NSQCDownload Sim ./play_program.nsqc 1310720 255

# 说明：
# ./NSQCDownload：程序名称
# Sim：逻辑名称。
# ./play_program.nsqc：配置NSQC的二进制文件路径
# 1310720：配置NSQC的二进制文件的大小（单位字节），NSQC文件默认通过通道0写入
```

### 数据流下发

```Bash
# 来到ivi/test/build目录下
- cd ~/ivi/test/build
## 命令：
- sudo ./streamDownload Sim 0 ./Sin_Fixed_10000MSps_0Offset_1Amp_100MHz_0MBw_0Phase_1024us_16bit_Signed_1024us_1Row_1Column.dat 20480000 4096

# 说明：
# ./streamDownload：程序名称
# Sim：逻辑名称。
# 1：波形下发的通道号 通道1：1；通道2：2
# ./Sin_Fixed_。。。_1Column.dat：波形的二进制文件路径
# 20480000：波形的二进制文件的大小（单位字节）
# 4096: start_addr(寄存器写入，读写DDR的起始地址)
```

测速：

![img](https://rigolportal.feishu.cn/space/api/box/stream/download/asynccode/?code=Njc3MWY4ZTQ5MTA1Y2EzYjMwYjFjMTE1ZGQ0ZTUyMjBfWGtIc1h1dGdpQ0lyV251WW94SFFIOUhVZURrTDNoR2hfVG9rZW46RHdrOWJ0YjdEb2VIc1h4Q01McWNTaHRNbnpoXzE3MjU1MjAwNDY6MTcyNTUyMzY0Nl9WNA)

### 系统停止（icd指令）

```Bash
# 来到ivi/test/build目录下
- cd ~/ivi/test/build
## 命令：
- sudo ./systemStop Sim

# 说明：
# ./systemStop：程序名称
# Sim：逻辑名称。
```
