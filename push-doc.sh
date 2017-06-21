#!/bin/sh

mkdir tmp
cd tmp
export GIT_DIR=../.git
git checkout master .
git checkout --detach
doxygen Doxyfile
export GIT_WORK_TREE=html
git symbolic-ref HEAD refs/heads/gh-pages
git add -A
git commit -m "documment update"
git push -u origin gh-pages
