# :mount_fuji:Hueco Predicto:crystal_ball:

This is a climbing difficulty classifier. It is developed for the paper _A fair grade: assessing climbing route difficulty through machine learning_ (Lindsay Kempen, 2019). The full text is available on [bachelorthesis.linths.com](http://bachelorthesis.linths.com).

## Run it

## Good to know

- Make sure to run the makefiles first. You have to `make` in `vomm/src` before running `vomm_train_all.sh` for example.
- [`human.txt`](phoenix/app/strangebeta/human.txt) contains human-readable versions of the four symbol sets made. For every symbol set, it contains the mapping of symbols to the climbing moves they represent. You can read a line in the file as follows: `<symbol-set> <symbol> <climbing-move>`.
- `make` in main folder is untested. It runs a chaotic generator for climbing routes, which is not needed for this project.
- Parsing
    1. Copy the `.gra` file right outside the Grammar folder.
    2. Run `./run_grammar`
    3. Drag the base.dic and simple.net to the Grammar folder, replace the old files
    4. Now you can parse by running `make`

## Paper abstract

In the sport of climbing and bouldering, difficulty grades are important for skill assessment, enjoyment and safety. Although there are standardized scales to minimize inconsistency, the difficulty assessment is performed by humans and sensitive to subjectivity. Therefore, we have developed a tool that predicts the difficulty grades of climbing routes using machine-learning.  
The routes are described in a semi-natural Domain-Specific Language, which can be parsed into symbol sequences. Here, a symbol represents a climbing move. The symbol sequences are then used as inputs to a variable-order Markov models (VOMMs) based classifier. With the VOMM prediction algorithm Decomposed Context Tree Weighting (DE-CTW), we trained one VOMM on _Easy_ climbing routes and one on _Hard_ climbing routes. By calculating a test route's likelihood for both VOMMs, the _average log-loss_}, we predict if a route is _Easy_ or _Hard_.  
We have implemented six predictor variations to vary with interpretation detail in the symbolization process. After using 50-fold cross validation on 146 climbing routes, our best performing variation performed slightly better than a trivial classifier. Still we believe this research's foundations are of interest for future research. We conclude with detailed explanations and proposed improvements.

Full text: [bachelorthesis.linths.com](http://bachelorthesis.linths.com)

## Structure

- `vomm` contains all the Variable-Order Markov Model related code.
  - `vmm` and `doc` contains/concerns code from Begleiter et al.
  - `data` contains the VOMMs generated using this code, using the climbing route input
- `data`

## Copyright

### Strange Beta

**TODO: GPL**

### Variable-order Markov Model prediction

Copyright (C) 2004  Ron Begleiter, Ran El-Yaniv, Golan Yona.  
Code from the paper "On Prediction Using Variable Order Markov Models"  
BibTeX: [http://www.cs.technion.ac.il/~rani/el-yaniv_bib.html#BegleiterEY04](http://www.cs.technion.ac.il/~rani/el-yaniv_bib.html#BegleiterEY04)

Trivial modifications are made by Lindsay Kempen (2019)  
2019-01-18: Change formatting of vmm/algs/DCTWPredictor.java  
2019-01-17: Change encoding of files in the vmm folder to UTF-8

This code is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version. This code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License ([GPL](http://www.gnu.org/copyleft/gpl.html)) for more details.

### Phoenix parser for spontaneous speech

Phoenix Spoken Language System is developed at Carnegie Mellon University.  
Paper: Ward, Wayne (1994): "Extracting information in spontaneous speech", In ICSLP-1994, 83-86.

### This project

**TODO: GPL**


## FAQ
