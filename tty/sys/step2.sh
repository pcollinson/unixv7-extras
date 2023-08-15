: Step 2 - install the files
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

echo '*** Step 2: Installing replacement files'
DEST=$SYS/dev
for name in dc.c dh.c dz.c kl.c tty.c; do
    if [ -f $DEST/orig/$name ]; then
	cp $name $DEST
    else
	echo "No backup for $DEST/$name - not installed"
    fi
done
echo "*** $SYS/dev updated"

DEST=$SYS/h
cp deftty.h $DEST
for name in tty.h pk.p; do
    if [ -f $DEST/orig/$name ]; then
	cp $name $DEST
    else
	echo "No backup for $DEST/$name - not installed"
    fi
done

if [ "$INCLUDE" != "" ]; then
    DEST=$INCLUDE
    for name in sgtty.h; do
	if [ -f $DEST/orig/$name ]; then
	    cp $name $DEST
	else
	    echo "No backup for $DEST/$name - not installed"
	fi
    done

    DEST=$INCLUDE/sys
    cp deftty.h $DEST
    for name in tty.h pk.p; do
	if [ -f $DEST/orig/$name ]; then
	    cp $name $DEST
	else
	    echo "No backup for $DEST/$name - not installed"
	fi
    done
fi
