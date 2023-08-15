: Make lost+found directory on named filesystem
SLOTS=20

for mntpt in "$@"; do
    if [ ! -d $mntpt ]; then
	echo "$mntpt is not a directory"
	exit 1
    fi
    if [ -d $mntpt/lost+found ]; then
	echo "$mntpt/lost+found exists"
	exit 1
    fi
    (   cd $mntpt/lost+found
	cnt=1
	while [ $cnt -le $SLOTS ]; do
	    touch fi$cnt
	    cnt=`expr $cnt + 1`
        done
	rm fi*
    )
done
