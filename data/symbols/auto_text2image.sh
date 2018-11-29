#!/bin/bash

if [[ "$OSTYPE" = "linux-gnu" ]]; then
	fonts_dir='/usr/share/fonts/'
else
	fonts_dir='/Library/Fonts/'
fi

echo "Fonts Dir: " $fonts_dir

text2image \
	--text symbols.txt \
	--outputbase Abyssinica \
	--font='Abyssinica SIL' \
	--fonts_dir $fonts_dir \
	--degrade_image=false \
	--rotate_image=false \
	--xsize=1800 \
	--ysize=1800 \
	--ptsize=14 \
	--box_padding 4 \
	# --glyph_resized_size 48 \
	# --output_individual_glyph_images \
