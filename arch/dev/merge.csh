#!/bin/csh

set x=0
set y=0

cp 3x3.ppm work.ppm

while ($y<3)
while ($x<3)
    @ f =$x + $y * 3 + 1
    @ y1 = $y * 24
    @ x1 = $x  * 24
    set filename = "big_wiz.$f""13.xpm"
    xpmtoppm $filename > tmp.ppm
    pnmpaste tmp.ppm $x1 $y1 work.ppm > tmp2.ppm
    mv tmp2.ppm work.ppm
    echo "x1 = $x1   y1=$y1 file = $filename"
    @ x ++
  end
  set x=0
  @ y ++
end
