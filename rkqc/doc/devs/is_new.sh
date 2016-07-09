cat $1 | grep "@since *1.3" | head -n1 | sed -e "s/.*/new features/g"
