#!/usr/bin/env bash

# For those distros that do not package bootloadHID
_qmk_install_bootloadhid() {
    if ! command -v bootloadHID > /dev/null; then
        wget https://www.obdev.at/downloads/vusb/bootloadHID.2012-12-08.tar.gz -O - | tar -xz -C /tmp
        pushd /tmp/bootloadHID.2012-12-08/commandline/ > /dev/null
        if make; then
            sudo cp bootloadHID /usr/local/bin
        fi
        popd > /dev/null
    fi
}

# No distros package sonixflasher yet
_qmk_install_sonixflasher() {
    if ! command -v sonixflasher > /dev/null; then
        latest_tag=$(curl -s https://api.github.com/repos/SonixQMK/SonixFlasherC/releases/latest | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')
        wget https://github.com/SonixQMK/SonixFlasherC/archive/refs/tags/${latest_tag}.tar.gz -O - | tar -xz -C /tmp
        pushd /tmp/SonixFlasherC-${latest_tag}/ > /dev/null
        if make; then
            sudo cp sonixflasher /usr/local/bin
        fi
        popd > /dev/null
    fi
}