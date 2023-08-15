: Step 3 - Validate files
: assuming we have a backup

if [ "$#" -eq 0 ]; then
    echo 'Call this script with the base address of your system source - perhaps /usr/sys'
    exit
fi
SYS=$1
: use this as a flag to suppress update of /usr/include
INCLUDE=""
if [ "$SYS" = "/usr/sys" ]; then
    INCLUDE=/usr/include
fi

echo "*** Step 3: Validating installed files"
echo "If you get any output then installation has failed"

DEST=$SYS/dev
for name in dc.c dh.c dz.c kl.c tty.c; do
    if [ ! -f $DEST/$name ]; then
	echo "Cannot find $DEST/$name"
    else
	cmp $name $DEST/$name
    fi
done

DEST=$SYS/h
for name in deftty.h tty.h pk.p; do
    if [ ! -f $DEST/$name ]; then
	echo "Cannot find $DEST/$name"
    else
	cmp $name $DEST/$name
    fi
done

if [ "$INCLUDE" != "" ]; then
    DEST=$INCLUDE
    for name in sgtty.h; do
	 if [ ! -f $DEST/$name ]; then
	     echo "Cannot find $DEST/$name"
	 else
	     cmp $name $DEST/$name
	 fi
    done
    DEST=$INCLUDE/sys
    for name in tty.h deftty.h pk.p; do
	 if [ ! -f $DEST/$name ]; then
	     echo "Cannot find $DEST/$name"
	 else
	     cmp $name $DEST/$name
	 fi
     done
fi
echo "Done"
