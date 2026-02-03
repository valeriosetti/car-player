#!/bin/bash

# Add root user to pulse-access group
if [ -e ${TARGET_DIR}/etc/group ]; then
    sed -i '/^pulse-access:/s/\(.*\)/\1,root/;s/:,/:/' ${TARGET_DIR}/etc/group
fi
