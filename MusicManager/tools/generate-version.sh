#!/bin/sh

set -eu

version_file=$1
output_file=$2
version=$(tr -d '[:space:]' < "$version_file")

case "$version" in
	*[!0-9.]*|.*|*.|*..*)
		echo "Invalid version '$version': expected MAJOR.MIDDLE.MINOR" >&2
		exit 1
		;;
esac

old_ifs=$IFS
IFS=.
set -- $version
IFS=$old_ifs
if [ "$#" -ne 3 ]; then
	echo "Invalid version '$version': expected MAJOR.MIDDLE.MINOR" >&2
	exit 1
fi

major=$1
middle=$2
minor=$3
mkdir -p "$(dirname "$output_file")"

sed \
	-e "s/@MAJOR@/$major/g" \
	-e "s/@MIDDLE@/$middle/g" \
	-e "s/@MINOR@/$minor/g" \
	-e "s/@VERSION@/$version/g" \
	tools/MusicManagerVersion.rdef.in > "$output_file"
