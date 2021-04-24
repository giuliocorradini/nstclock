I can't provide the fonts sources since I haven't got the license to do so.
You should download the following fonts from the web and parse them using
the provided script:

- CD-IconsPC.ttf from www.pixietype.com
- Animal-Crossing-Wild-World.ttf
- MotorolaScreentype.ttf
- Mario64.ttf from fontstruct.com
- zeldadxt.ttf by Brian Kent
- radiospacebitmap.ttf by Daniel Zarodozny from www.iconian.com
- Mario-Kart-DS.ttf

Make sure you initiated `git submodule` and downloaded the ssd1306 library in the
components directory.

Populate this folder with your fonts in ttf format, cd to the root directory
of the project and run the following script:

```bash
./fonts/convert.sh
```
