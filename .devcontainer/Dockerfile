FROM ubuntu

RUN apt-get -y update

# ----------------------------------------------
# tips fra https://code.visualstudio.com/docs/remote/containers-advanced#_adding-a-nonroot-user-to-your-dev-container
# ----------------------------------------------
ARG USERNAME=rolfn
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME -s /bin/bash \
    #
    # [Optional] Add sudo support. Omit if you don't need to install software after connecting.
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# ********************************************************
# * Anything else you want to do like clean up goes here *
# ********************************************************

# [Optional] Set the default user. Omit if you want to keep the default as root.
# this seems to affect all later commands (apt-get fails with permission error)
# USER $USERNAME

# ------------------------------------------------------------------------------
# Set Environment
# ------------------------------------------------------------------------------

# Common
ENV SMING_HOME /opt/Sming/Sming
ENV PYTHON /usr/bin/python3

# Esp8266
#ENV UDK_ROOT /opt/esp-open-sdk
ENV EQT_ROOT /opt/esp-quick-toolchain
ENV ESP_HOME $EQT_ROOT

# Esp32
ENV IDF_PATH /opt/esp-idf
ENV IDF_TOOLS_PATH /opt/esp32
ENV ESP32_PYTHON_PATH $PYTHON

# ------------------------------------------------------------------------------
# Pre-requisites
# ------------------------------------------------------------------------------

RUN DEBIAN_FRONTEND=noninteractive \
    TZ=Europe/London \
    apt-get install -y git tzdata

# ------------------------------------------------------------------------------
# Fetch Sming and install tools
# ------------------------------------------------------------------------------

ARG SMING_BRANCH=develop
ARG SMING_REPO=SmingHub/Sming
ARG INSTALL_ARGS=all

RUN mkdir -p /opt && chown $USERNAME:$USERNAME /opt

USER $USERNAME

RUN git clone -b $SMING_BRANCH -- https://github.com/$SMING_REPO $(readlink -m $SMING_HOME/..)

WORKDIR /tmp

RUN $SMING_HOME/../Tools/install.sh $INSTALL_ARGS
