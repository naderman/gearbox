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

TARBALL=gearbox-release-doc.tar.gz

force ls
force doxyorca doxyfile
force cd html
force tar --exclude=$TARBALL -zcvf $TARBALL *
force scp $TARBALL shell.sourceforge.net:

ssh shell.sourceforge.net "cd /home/groups/g/ge/gearbox/htdocs/; mv ~/$TARBALL .; tar -zxvf $TARBALL"
