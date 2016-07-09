cat $1 | grep "@brief" | head -n1 | sed -e "s/.*@brief *//"
