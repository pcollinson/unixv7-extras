: Script to edit the system files that require removal or replacing
: old files are placed in the appropriate orig directory
: For safety the script should be supplied with the name of the
: system file tree

: First see if /bin/[ exists
: It did not on my distribution
if ! test -f '/bin/['; then
    echo 'The file /bin/[ is needed to support shell scripting'
    echo 'and needs creating on this system, please execute:'
    echo 'cd /bin'
    echo 'ln test ['
    exit
fi
if [ "$#" -eq 0 ]; then
    echo 'Call this script with the base address of your system source - perhaps /usr/sys'
    exit
fi
SYS=$1
: use this as a flag to suppress update of /usr/include
INCLUDE=""

echo '*** Step 1: make orig directories and copy backup files'
for name in dev/orig h/orig; do
    if [ ! -d $SYS/$name ]; then
	echo "Making $SYS/$name"
	mkdir $SYS/$name
	if [ ! -d $SYS/$name ]; then
	    echo "Failed to make $SYS/$name - stopping"
	    exit
	fi
    fi
done

if [ "$SYS" = "/usr/sys" ]; then
    INCLUDE="/usr/include"
    for name in $INCLUDE/orig $INCLUDE/sys/orig; do
	if [ ! -d $name ]; then
	    echo "Making $name"
	    mkdir $name
	    if [ ! -d $name ]; then
		echo "Failed to make $name - stopping"
		exit
	    fi
	fi
    done
else
    echo "Installing in $SYS and not /usr/sys"
    echo "For safety, not updating /usr/include"
fi

: File backup
: sys/dev
DEST=$SYS/dev/orig
for name in dc.c dh.c dz.c kl.c tty.c; do
    if [ ! -f $DEST/$name ]; then
	echo "Backup orig/$name to $DEST/$name"
	cp orig/$name $DEST/$name
	if [ ! -f $DEST/$name ]; then
	    echo "Failed to copy $name"
	    exit
	fi
    fi
done

: sys/h
DEST=$SYS/h/orig
for name in tty.h pk.p; do
    if [ ! -f $DEST/$name ]; then
	echo "Backup orig/$name to $DEST/$name"
	cp orig/$name $DEST/$name
	if [ ! -f $DEST/$name ]; then
	    echo "Failed to copy $name"
	    exit
	fi
    fi
done

: include
if [ "$INCLUDE" != "" ]; then
    DEST=$INCLUDE/orig
    for name in sgtty.h; do
	if [ ! -f $DEST/$name ]; then
	    echo "Backup orig/$name to $DEST/$name"
	    cp orig/$name $DEST/$name
	    if [ ! -f $DEST/$name ]; then
		echo "Failed to copy $name"
		exit
	    fi
	fi
    done
    DEST=$INCLUDE/sys/orig
    for name in tty.h pk.p; do
	if [ ! -f $DEST/$name ]; then
	    echo "Backup orig/$name to $DEST/$name"
	    cp orig/$name $DEST/$name
	    if [ ! -f $DEST/$name ]; then
		echo "Failed to copy $name"
		exit
	    fi
	fi
    done
fi
exit
