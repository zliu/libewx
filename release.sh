#! /bin/sh

VERSION_FILE=ewx_version.h

version=`grep SOFTWARE_VERSION ${VERSION_FILE} | sed -e "s/.*\"Version \(.*\)\".*/\1/"`
build=`grep BUILD ${VERSION_FILE} | sed -e "s/.*\"build \(.*\)\".*/\1/"`

if [ $# -eq 3 ]
then
    if [ $1 == "version" ]
    then
        git tag | grep $2 > /dev/null
        if [ $? -ne 1 ]
        then
            echo "version${2} existed!"
            exit 1
        fi
        newversion=$2
        sed -e "s/Version\ ${version}/Version\ ${newversion}/" ${VERSION_FILE} > version.$$
        mv version.$$ ${VERSION_FILE}
    else
        echo "Usage: release [version x.x.x] message"
        exit 1
    fi
else
    if [ $# -ne 1 ]
    then
        echo "Usage: release [version x.x.x] message"
        exit 1
    fi
fi

newbuild=`printf %04d $( expr $build + 1 )`
sed -e "s/build\ ${build}/build\ ${newbuild}/" ${VERSION_FILE} > version.$$
mv version.$$ ${VERSION_FILE}

git add ${VERSION_FILE}

if [ $# -eq 3 ]
then
    git commit -m $3
    git tag "v${newversion}"
else
    git commit -m $1
fi
git tag "${newbuild}"
