dhcp
sleep 1
setenv serverip 192.168.3.100

setenv kernel_filename "Image"
setenv fdt_filename "meson-gxbb-odroidc2.dtb"
setenv bootargs "console=ttyAML0,115200n8 root=/dev/nfs ip=dhcp nfsroot=${serverip}:/run/user/1000/nfs,tcp,nfsvers=3 earlyprintk rw rootwait usbcore.autosuspend=-1 fbcon=map:2"

echo > Loading Kernel...
tftp ${kernel_addr_r} ${kernel_filename}
echo > Loading FDT...
tftp ${fdt_addr_r} ${fdt_filename}

echo > Booting System...
booti ${kernel_addr_r} - ${fdt_addr_r}
