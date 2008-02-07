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

TARBALL=hydro-release-doc.tar.gz

force ls
force doxyorca doxyfile
force cd html
force tar --exclude=$TARBALL -zcvf $TARBALL *
force scp $TARBALL acfr@shell.sourceforge.net:

ssh acfr@shell.sourceforge.net "cd /home/groups/o/or/orca-robotics/htdocs/hydro; mv ~/$TARBALL .; tar -zxvf $TARBALL"
