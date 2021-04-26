#!/bin/bash

function activate_venv {
    if [[ "$VIRTUAL_ENV" == "" ]]; then
        source ./venv/bin/activate
    fi
}

function generate_big {
    cd fonts
    for font in $(ls *.ttf); do
        if [[ "$font" =~ ^big_* ]]; then
            echo $font
        else
            cp "$font" "big_$(basename $font .ttf).ttf"
        fi
    done
    cd ..
}

function clean_big {
    cd fonts
    for font in $(ls big_*); do
        rm $font
    done
    cd ..
}

function clean_dashes {
    cd fonts
    for font in $(ls *.ttf); do
        mv "$font" "$(echo "$font" | tr -d '-')"
    done
    cd ..
}

function convert_ttf {
    for font in $(ls fonts/*.ttf); do
        echo "Converting $font"
        header="$(basename $font .ttf).h"
        if [[ "$font" =~ ^fonts/big_* ]]; then
            python3 ./fonts/lcdgfx/tools/fontgenerator.py --ttf "$font" -f new -s 20 > "$header"
        else
            python3 ./fonts/lcdgfx/tools/fontgenerator.py --ttf "$font" -f new -s 12 > "$header"
        fi
        mv "$header" "./lib/fonts/"
    done
}

if [[ "$VIRTUAL_ENV" == "" ]]; then
    activate_venv
fi

if [[ "$1" == "--bigfonts" ]]; then
    generate_big
fi

clean_dashes
convert_ttf

if [[ "$1" == "--bigfonts" ]]; then
    clean_big
fi