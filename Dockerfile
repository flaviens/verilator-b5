FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive

# Install apt dependencies
RUN apt-get update && apt-get install build-essential git help2man perl python3 make autoconf flex bison ccache libgoogle-perftools-dev numactl perl-doc libfl2 libfl-dev zlib1g zlib1g-dev g++-9 -y
# Use g++9 by default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 --slave /usr/bin/g++ g++ /usr/bin/g++-9

# Install latest Verilator
RUN bash -c "git clone https://github.com/verilator/verilator && cd verilator && autoconf && ./configure && make -j$(nproc) && make install"
RUN mkdir -p /include

# Copy the source code
COPY tb.cc /tb.cc
COPY top.sv /top.sv

COPY ticks.h /include/ticks.h
COPY interface_sizes.h /include/interface_sizes.h
COPY inputs.txt /inputs.txt

ENV SIMLEN 6
ENV TRACEFILE /trace.vcd

RUN verilator --cc --exe --Wno-UNOPTFLAT --build tb.cc top.sv -CFLAGS '-I/include -g' --Mdir /obj_dir --build-jobs 32
RUN verilator --cc --exe --trace --Wno-UNOPTFLAT --build tb.cc top.sv -CFLAGS '-I/include -g' --Mdir /obj_dir_traces --build-jobs 32

RUN ./obj_dir/Vtop > notrace_out.log
RUN ./obj_dir_traces/Vtop > withtrace_out.log
