# using codesourcery arm-2009q3-67
export PATH=${PATH}:~/arm-2009q3/bin/
export CROSS_COMPILE=arm-none-eabi-
export TOOLCHAIN_PREFIX=arm-none-eabi-
export AS=${CROSS_COMPILE}as
export OBJCOPY=${CROSS_COMPILE}objcopy

make msm7200a_htc_wince clean
make msm7200a_htc_wince

cp ./build-msm7200a_htc_wince/lk.bin .
cp ./build-msm7200a_htc_wince/lk.bin ./tools/

pushd ./tools/
$AS tinboot.S -o tinboot.o
$OBJCOPY tinboot.o -O binary tinboot
gcc generate.c -o generate
./generate
yang -F lk.nbh -f lk.nb -t 0x400 -s 64 -d KOVS***** -c 11111111 -v 1.0.XDAPOOP -l WWE
cp lk.nbh ../kovsimg.nbh
rm lk.bin
popd
