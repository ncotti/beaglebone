set architecture arm
target extended-remote 192.168.7.2:2159
set remote exec-file /home/debian/exe.elf
lay src
b main
run
