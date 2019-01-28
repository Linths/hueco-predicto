# :mount_fuji:Hueco Predicto:crystal_ball:

A fair grade: assessing climbing route difficulty through machine learning

## Good to know

- Make sure to run the makefiles first. You have to `make` in `vomm/src` before running `vomm_train_all.sh` for example.
- [`human.txt`](phoenix/app/strangebeta/human.txt) contains human-readable versions of the four symbol sets made. For every symbol set, it contains the mapping of symbols to the climbing moves they represent. You can read a line in the file as follows: `<symbol-set> <symbol> <climbing-move>`.
- `make` in main folder is untested. It runs a chaotic generator for climbing routes, which is not needed for this project.
- Parsing
    1. Copy the `.gra` file right outside the Grammar folder.
    2. Run `./run_grammar`
    3. Drag the base.dic and simple.net to the Grammar folder, replace the old files
    4. Now you can parse by running `make`