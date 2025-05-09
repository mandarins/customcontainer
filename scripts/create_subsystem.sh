#!/bin/bash

# Check for exactly 2 parameters
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <base_folder> <hostname>"
    exit 1
fi

BASE_FOLDER="$1"
HOSTNAME="$2"
CHROOT_FLAG="$3"
TARGET_DIR="$BASE_FOLDER/$HOSTNAME"

# Create the hostname directory if it doesn't exist
mkdir -p "$TARGET_DIR"

# Create the required subdirectories
for dir in bin etc lib lib64 proc usr; do
    mkdir -p "$TARGET_DIR/$dir"
done

mkdir -p "$TARGET_DIR/usr/libexec/sudo"


cp /lib/x86_64-linux-gnu/libproc2.so.0  "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libc.so.6   "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libsystemd.so.0   "$TARGET_DIR/lib"
cp  /lib/x86_64-linux-gnu/libcap.so.2   "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libgcrypt.so.20   "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/liblz4.so.1  "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/liblzma.so.5   "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libzstd.so.1   "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libgpg-error.so.0   "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libtinfo.so.6   "$TARGET_DIR/lib"
cp /lib64/ld-linux-x86-64.so.2   "$TARGET_DIR/lib64"
cp /lib/x86_64-linux-gnu/libselinux.so.1   "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libpcre2-8.so.0   "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libmount.so.1  "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libblkid.so.1   "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libaudit.so.1   "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libapparmor.so.1 "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libaudit.so.1 "$TARGET_DIR/lib64"
cp /lib/x86_64-linux-gnu/libcrypto.so.3 "$TARGET_DIR/lib"
cp /lib/x86_64-linux-gnu/libcap-ng.so.0 "$TARGET_DIR/lib"
cp /usr/libexec/sudo/libsudo_util.so.0   "$TARGET_DIR/usr/libexec/sudo"

cp /bin/bash   "$TARGET_DIR/bin"
cp /bin/ls   "$TARGET_DIR/bin"
cp /bin/cp   "$TARGET_DIR/bin"
cp /bin/mkdir   "$TARGET_DIR/bin"
cp /bin/rm   "$TARGET_DIR/bin"
cp /bin/echo   "$TARGET_DIR/bin"
cp /bin/ps   "$TARGET_DIR/bin"
cp /bin/mount   "$TARGET_DIR/bin"
cp /bin/hostname   "$TARGET_DIR/bin"
cp /bin/sleep     "$TARGET_DIR/bin"
cp /bin/cat       "$TARGET_DIR/bin"
cp /bin/umount   "$TARGET_DIR/bin"
cp /bin/sudo   "$TARGET_DIR/bin"

cp /etc/fstab   "$TARGET_DIR/etc"
cp /etc/ld.so.cache   "$TARGET_DIR/etc"
cp /etc/ld.so.conf   "$TARGET_DIR/etc"
cp /etc/locale.gen   "$TARGET_DIR/etc"
cp /etc/locale.conf   "$TARGET_DIR/etc"
cp /etc/nsswitch.conf   "$TARGET_DIR/etc"
cp /etc/resolv.conf   "$TARGET_DIR/etc"
cp /etc/hostname   "$TARGET_DIR/etc"
cp /etc/hosts   "$TARGET_DIR/etc"
chmod -R +x "$TARGET_DIR/bin"

# ...existing code...
echo "$HOSTNAME" > "$TARGET_DIR/etc/hostname"
# ...existing code...
echo "Folders created under $TARGET_DIR:"
ls -1 "$TARGET_DIR"

# If third parameter is 'chroot', mount /proc and chroot
if [ "$CHROOT_FLAG" == "chroot" ]; then
    sudo mount --bind /proc "$TARGET_DIR/proc"
    echo "Entering chroot environment. Type 'exit' to leave."
    ##sudo chroot "$TARGET_DIR" /bin/bash
    sudo chroot "$TARGET_DIR" /bin/bash -c "hostname $HOSTNAME; exec /bin/bash"
    sudo umount "$TARGET_DIR/proc"
fi
