REM Formats images so you can use them as screensavers
REM Put your black-and-white 300x400 JPEG images in /screensavers on the uSD card
REM Needs ImageMagick installed
REM Run in a directory filled with JPEG images
REM I got some from https://multicolor.io/

REM Resize to at least a height of 400, width may be greater then 300
mogrify -resize x400 *.jpg

REM Crop to 300x400 (height won't change) so that the image is 300 pixels wide
mogrify -crop 300x400+0+0 -gravity center *.jpg

REM Monochrome everything
mogrify -monochrome *.jpg
