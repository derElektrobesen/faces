#!/bin/sh

rm -f report.bbl
texi2pdf report.tex && texi2pdf report.tex
