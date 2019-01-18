# 🔮🏔 Hueco Predicto

A fair grade: assessing climbing route difficulty through machine learning

## Good to know

- Make sure to run the makefiles first. You have to `make` in `vomm/src` before running `vomm_train_all.sh` for example.
- `phoenix/app/strangebeta/symbolic.txt` contains the mapping of the symbols to moves of the data set. The eventual symbols are numbers. There are four symbol sets, and in different symbol sets a number has a different meaning. To find out what a number means, you'll need to comparing the file with the input data for now.
- `make` in main folder is untested. It runs a chaotic generator for climbing routes, which is not needed for this project.