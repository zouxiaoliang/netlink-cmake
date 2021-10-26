#!/usr/bin/env bash
SOURCE="$0"

while [ -h "$SOURCE"  ]; do 
    DIR="$( cd -P "$( dirname "$SOURCE"  )" && pwd  )"
    SOURCE="$(readlink "$SOURCE")"
	# if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
    [[ $SOURCE != /*  ]] && SOURCE="$DIR/$SOURCE" 
done
DIR="$( cd -P "$( dirname "$SOURCE"  )" && pwd  )"

kernel_version=`uname -r` 

function version_check() {
    local match_result=`uname -r | grep $1 | wc -l`
    if [ "$match_result" == "1" ] ;  then
        return 0
    else
        return 1
    fi
    # return $match_result
}

libnl_source=libnl-source

rm -rf libnl
rm -rf ${libnl_source}

function build_libnl() {
    local libnl_path=deps/libnl-1.0-pre6.tar.gz
    
    if [ ! -f ${libnl_path} ] ; then 
        exit 1
    fi 
    
    rm -rf ${libnl_source} && mkdir ${libnl_source} && tar -xvf ${libnl_path} -C ${libnl_source} --strip-components 1
    if [ $? != 0 ] ; then 
        echo "=> decompress failed!!!"
        exit 1
    fi 

    cd ${libnl_source} && ./configure --prefix=${DIR}/libnl && make && make install 
    cd ${DIR}
}

mkdir -p handle

if version_check "2.6.18" ; then
    echo "=> kernel verson: ${kernel_version} using libnl-1.0.pre6"
    echo "=> build libnl: deps/libnl-1.0-pre6.tar.gz"
    build_libnl deps/libnl-1.0-pre6.tar.gz

    echo "=> configure netlink wrapper library ..."
    cp -r versions/1.0-pre6/* handle
elif version_check "2.6.32" || version_check "4.4.0" || version_check "3.10.0" || version_check "3.16.0" || version_check "4.15.0"; then
    echo "=> kernel verson: ${kernel_version} using system netlink"
    echo "=> configure netlink wrapper library ..."
    cp -f versions/kernel-2.8.32/* handle
else
    echo "other" 
fi

echo "=> next step: cmake . && make "