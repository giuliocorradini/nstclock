#!/bin/bash

function activate_venv {
    if [[ "$VIRTUAL_ENV" == "" ]]; then
        source ./venv/bin/activate
    fi
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
        python3 ./fonts/lcdgfx/tools/fontgenerator.py --ttf "$font" -f new -s 12 > "$header"
        mv "$header" "./lib/fonts/"
    done
}

clean_dashes
convert_ttf