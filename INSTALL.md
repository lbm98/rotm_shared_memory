## Environment

```bash
PROJECT_DIR=`pwd`
mkdir -p network
```

## GNB

```bash
cd $PROJECT_DIR/network

mkdir srsRAN_Project
cd srsRAN_Project
git -c advice.detachedHead=false \
clone --depth 1 --branch release_23_10_1 https://github.com/srsran/srsRAN_Project ./

mkdir build
cd build

cmake .. \
-G Ninja \
-DENABLE_EXPORT=ON \
-DENABLE_ZEROMQ=ON

cmake --build . --target gnb
```

## Core

```bash
cd $PROJECT_DIR/network/srsRAN_Project/docker
sudo docker compose build 5gc
```

## UE

```bash
cd $PROJECT_DIR/network

mkdir rotm_srsRAN_4G_fork
cd rotm_srsRAN_4G_fork
git clone https://github.com/lbm98/rotm_srsRAN_4G_fork ./
git checkout fuzz
```

Create a new CLion project.
Build targets: `srsue` and `srsran_rf_zmq`.

```bash
cd $PROJECT_DIR/network/rotm_srsRAN_4G_fork

git clone https://github.com/lbm98/rotm_shared_memory
```

## Fuzzer

```bash
cd $PROJECT_DIR/fuzzer

git clone https://github.com/lbm98/rotm_shared_memory
```