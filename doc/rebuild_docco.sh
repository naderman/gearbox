#!/bin/sh

force () {
    echo "$*"
    $*

    status=$?

    if [ $status -ne 0 ]; then
        echo "Problem executing '$*'"
        exit 1
    else
        echo "Executed '$*' OK"
    fi
}

# doxygen
DOXYGENCMD=/usr/bin/doxygen
DOXYFILE=doxyfile
TARBALL=gearbox-release-doc.tar.gz

force ls
force $DOXYGENCMD $DOXYFILE
force cd html
force tar --exclude=$TARBALL -zcvf $TARBALL *
force scp $TARBALL shell.sourceforge.net:

ssh shell.sourceforge.net "cd /home/groups/g/ge/gearbox/htdocs/; mv ~/$TARBALL .; tar -zxvf $TARBALL"
