FROM ubuntu:22.04

# Install yarp prerequisites
RUN apt update && apt install -y wget unzip \
    build-essential git cmake-curses-gui swig \
    libeigen3-dev \
    libace-dev \
    libedit-dev \
    libsqlite3-dev \
    libtinyxml-dev \
    qtbase5-dev qtdeclarative5-dev qtmultimedia5-dev \
    qml-module-qtquick2 qml-module-qtquick-window2 \
    qml-module-qtmultimedia qml-module-qtquick-dialogs \
    qml-module-qtquick-controls qml-module-qt-labs-folderlistmodel \
    qml-module-qt-labs-settings \
    libqcustomplot-dev \
    libgraphviz-dev \
    libjpeg-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-libav \
    python3.10-dev \
    pip \
    curl \
    zip \
    sudo && \
    rm -rf /var/lib/apt/lists/*

RUN useradd -l -G sudo -md /home/user -s /bin/bash -p user user && \
    sed -i.bkp -e 's/%sudo\s\+ALL=(ALL\(:ALL\)\?)\s\+ALL/%sudo ALL=NOPASSWD:ALL/g' /etc/sudoers

USER user
WORKDIR /home/user

# Install ycm
RUN git clone https://github.com/robotology/ycm.git && mkdir robotology && mv ycm/ robotology/ \
    && cd robotology/ycm && mkdir build \
    && cd build && cmake .. && make && sudo make install && \
    rm -rf /ycm

ENV PYTHONPATH=/usr/local/lib/python3/dist-packages

# Install yarp
RUN cd robotology && git clone https://github.com/robotology/yarp.git && \
    cd yarp && \
    mkdir build && cd build \ 
    && cmake .. \
    && make -j8 && sudo make install && \
    rm -rf /yarp

# Install llamacpp library
RUN git clone https://github.com/ggerganov/llama.cpp.git && \
    cd llama.cpp && cmake -B build && cmake --build build --config Release

# Compile and install llama2 device
RUN cd /home/user && git clone https://github.com/robotology-playground/yarp-device-llama2.git && \
    cd yarp-device-llama2 && cmake -S. -Bbuild -DCMAKE_INSTALL_PREFIX=/home/leonardo/Repos/yarp-device-llama2/install && \
    cmake --build build && cmake --build build --target install 

    

