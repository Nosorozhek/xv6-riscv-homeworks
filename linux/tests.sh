#!/bin/bash

Color_Off='\033[0m'
BRed='\033[1;31m'
BGreen='\033[1;32m'
BYellow='\033[1;33m'

echo -e "\n${BYellow}=== Setting up environment ====${Color_Off}"

make || exit 1
touch ext2.img
truncate --size 30M ext2.img
sudo mkfs.ext2 ext2.img "$@" || exit 1
sleep 0.2

mkdir ext2fs
sudo mount -o loop ext2.img ext2fs

sleep 0.1

directories=(directory directory/for_medium large_file_is_here only_sparse_file empty_folder)
files=(small directory/for_medium/medium large_file_is_here/large only_sparse_file/sparse)
inodes=()
checksums=()

for directory in ${directories[*]}
do
  sudo mkdir ext2fs/$directory
  sudo chmod 777 ext2fs/$directory
done


for file in ${files[*]}
do
  sudo touch ext2fs/$file
  sudo chmod 777 ext2fs/$file
done

sudo head -c 6 /dev/urandom > ext2fs/${files[0]}    # small
sudo head -c 22K /dev/urandom > ext2fs/${files[1]}  # medium
sudo head -c 20M /dev/urandom > ext2fs/${files[2]}  # large
sudo truncate -s 1G ext2fs/${files[3]}              # sparse

sleep 0.1


for file in ${files[*]}
do
  inode=$(ls -ali ext2fs/$file | grep -o "^[0-9]*")
  inodes+=($inode)
  checksum=$(sha512sum ext2fs/$file | grep -o "^[0-9a-f]*")
  checksums+=($checksum)
done

sudo umount ext2fs/ || exit 1
sudo rm -rf ext2fs/
sleep 0.1


echo -e "\n${BYellow}=== Testing with filesystem image ====${Color_Off}"

passed=0
failed=0
for i in ${!files[*]}
do
  file=${files[$i]}
  inode=${inodes[$i]}
  checksum=${checksums[$i]}
  printf "\n----------\n"
  printf "Test %s\n" $file
  printf "Inode: %d\n" $inode

  sha=$(./inode ext2.img $inode | sha512sum | grep -o "^[0-9a-f]*")
  if [ "$sha" = "$checksum" ]
  then
    echo "sha512sum matches"
    echo -e "${BGreen}---Test passed---${Color_Off}"
    passed=$((passed+1))
  else
    echo "sha512sum doesn't match"
    echo "Actual:   $sha"
    echo "Expected: $checksum"
    echo -e "${BRed}---Test failed---${Color_Off}"
    failed=$((failed+1))
  fi
done

printf "\n----------\n"
echo "Passed: $passed"
echo "Failed: $failed"

echo -e "\n${BYellow}=== Testing with loop device ====${Color_Off}"

LOOP_DEV=$(sudo losetup -f)
if [ -z "$LOOP_DEV" ]; then
  echo -e "${BRed}No available loop device found!${Color_Off}"
  exit 1
fi

sudo losetup $LOOP_DEV ext2.img

echo "Loop device information:"
sudo losetup -a | grep ext2.img
echo "Block device details:"
lsblk -o name,size,fstype | grep $(basename $LOOP_DEV)

passed_loop=0
failed_loop=0

for i in ${!files[*]}
do
  file=${files[$i]}
  inode=${inodes[$i]}
  checksum=${checksums[$i]}
  printf "\n----------\n"
  printf "Test %s\n" $file
  printf "Inode: %d\n" $inode

  sha=$(sudo ./inode $LOOP_DEV $inode | sha512sum | grep -o "^[0-9a-f]*")
  if [ "$sha" = "$checksum" ]
  then
    echo "sha512sum matches"
    echo -e "${BGreen}---Test passed---${Color_Off}"
    passed_loop=$((passed_loop+1))
  else
    echo "sha512sum doesn't match"
    echo "Actual:   $sha"
    echo "Expected: $checksum"
    echo -e "${BRed}---Test failed---${Color_Off}"
    failed_loop=$((failed_loop+1))
  fi
done

printf "\n----------\n"
echo "Passed: $passed_loop"
echo "Failed: $failed_loop"

sudo losetup -d $LOOP_DEV
rm -rf ext2.img
make clean

echo -e "\n${BYellow}=== Final Test Summary ====${Color_Off}"
echo "Image file tests - Passed: $passed, Failed: $failed"
echo "Loop device tests - Passed: $passed_loop, Failed: $failed_loop"

total_passed=$((passed + passed_loop))
total_failed=$((failed + failed_loop))
total_tests=$((total_passed + total_failed))

if [ $total_failed -eq 0 ]; then
  echo -e "${BGreen}All $total_tests tests passed!${Color_Off}"
else
  echo -e "${BRed}$total_failed of $total_tests tests failed!${Color_Off}"
fi
