#!/bin/bash
cur_pwd=$(pwd)
i=0
line=
max_insert=499
total=0
t_names=0

db_path=$cur_pwd/database/faces.db3

insert() {
    echo "insert into test_images(path, name_id) values $1;" | sqlite3 $db_path
    local i=$2
    total=$((total + i))
    echo "$2 images added [$total total]";
}

cd images/ExtendedYaleB
cur_pwd=$(pwd)
while read folder; do
    row_id=$(echo "insert into neuro_networks(name) values ('$folder'); select last_insert_rowid();" | sqlite3 $db_path)
    t_names=$((t_names + 1))

    while read file; do
        if [[ $i != 0 ]]; then
            line="$line, "
        fi
        line="$line('$cur_pwd/$file', $row_id)"

        if [ "$i" -ge "$max_insert" ]; then
            insert "$line" $i
            i=0
            line=
        else
            i=$((i + 1))
        fi
    done < <(find $folder -type f -name "*.pgm")
done < <(find . -type d)

if [ "$i" -ne "0" ]; then
    insert "$line" $i
fi

echo "Total $total images of $t_names people added"
