#!/bin/sh
# A tour of toybox on the Linux kernel compatibility layer. Twister runs it
# through the console harness and matches the output of every step in order;
# it ends in the interactive shell.
echo "hello, world!" | sed -e "s|h|j|" | tee /tmp/foo.txt
uname -a
hostname
id -u
id -g
basename /tmp/foo.txt
dirname /tmp/foo.txt
file /bin/toybox
file /
file /tmp/foo.txt
t=$(mktemp)
ls /tmp
rm "$t"
(cat /tmp/foo.txt; cat /tmp/foo.txt) | tail -n 1
ln -sf /tmp/foo.txt /tmp/foo
ls -la /tmp
realpath /tmp/foo
find /tmp -name 'foo*' | sort
mkfifo /tmp/fifo
ls -l /tmp/fifo
cat /tmp/fifo & echo "through the fifo" > /tmp/fifo
wait
mknod /tmp/pipe p
ls -l /tmp/pipe
seq 10000 | head -2
echo /bin/s* | wc -w
ping -c 3 localhost
ping6 -c 3 localhost
sh -c 'kill -TERM $$'
echo "signal status $?"
echo "demo complete"
exec sh -i
