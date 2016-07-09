#!/bin/bash

for i in html/*.html
do
    sed $i -i -e "s@\(Page</span></a></li>\)@\\1<li><a href=\"overview.html\"><span>Overview</span></a></li>@"
done

sed html/overview.html -i -e "s@ class=\"current\"@@"
sed html/overview.html -i -e "s@<li><a href=\"overview.html\">@<li class=\"current\"><a href=\"overview.html\">@"
