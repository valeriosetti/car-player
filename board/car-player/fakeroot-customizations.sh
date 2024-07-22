#!/bin/bash

# Add root user to pulse-access group
if [ -e ${TARGET_DIR}/etc/group ]; then
    sed -i '/^pulse-access:/s/\(.*\)/\1,root/;s/:,/:/' ${TARGET_DIR}/etc/group
fi

# Set ed25519 private and public keys file permissions
if [ -e ${TARGET_DIR}/etc/ssh/ssh_host_ed25519_key ]; then
    chmod go-rw ${TARGET_DIR}/etc/ssh/ssh_host_ed25519_key
fi
if [ -e ${TARGET_DIR}/etc/ssh/ssh_host_ed25519_key.pub ]; then
    chmod go-rw ${TARGET_DIR}/etc/ssh/ssh_host_ed25519_key.pub
fi
