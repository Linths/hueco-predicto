# :mount_fuji:Hueco Predicto:crystal_ball:

This is a climbing difficulty classifier, developed as part of a research paper.  
:page_facing_up: _A fair grade: assessing difficulty of climbing routes through machine learning_ (Lindsay Kempen, 2019)  
:books: Full text via [bachelorthesis.linths.com](http://bachelorthesis.linths.com)

All data used in this work (in-kind and anonymized), and some of the underlying code, was provided  by the [Strange Beta](http://strangebeta.org) project by [Caleb Phillips](http://smallwhitecube.com).

Please message me if you have any questions or remarks!

## Run it
**TO DO**

Recommended settings
- **Model depth = 5**  
Used for the VOMM learning process. This is the max length of subsequences (_context_, here: number of consecutive climbing holds/moves) considered for the probability calculation.
- **Identifier**  
Determines save location for VOMMs ([vomm/src/data/grades](vomm/src/data/grades)/\<identifier\>). A program run erases the existing folder contents first, so use a new ID if you want to keep the old contents.
- **k ∈ {10..30}**  
A high k can take hours, but it can benefit the accuracy with this small of a dataset.
- **N <= data_size**  
A high N can take hours. Choose a N of -1 to set N to data_size.


Commands
- k-fold cross-validation  
  `ruby performance_kfold.rb <k> <model-depth> <id>`
- "single classification"
  `ruby performance_one.rb <N> <model-depth> <id>`
- 

## Good to know

**TO DO: Clean up, add relevant parts to [Run it](#Run-it) & [Important files](#Important-files)**
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

## Code structure

- `vomm`: Variable-Order Markov Model
  - `vmm` & `doc`: code & documentation from Begleiter et al.
  - `data`: VOMMs generated using this code, using the climbing route input
- `phoenix`: parser
  - `PhoenixLib` & `Doc` code & documentation from Wayne Ward / CMU
  - `app/strangebeta`: parser grammar, input, output and extra parsing steps (e.g. input sanitation, symbolization). Written by Philips et al. Modifications & additions by Lindsay Kempen
- `data`
**TO DO**

## Important files

- `human.txt`
- **TO FILL**

## Copyright

### Strange Beta: a climbing route assistance system

The Strange Beta system is a large contribution to this project. The Strange Beta components included in this project are listed below.. Several files have been modified and (adapted) parts have been reused for new files.

- Data: climbing routes expressed in a semi-natural DSL
- Climbing route generator (_included but not used_)
- Climbing route parsing & symbolization
- Climbing route interpolation using Variable-Order Markov Models

For more information on [Strange Beta](http://strangebeta.org), please reference "[strange beta: an assistance system for indoor rock climbing route setting](https://aip.scitation.org/doi/10.1063/1.3693047)" published in the AIP Chaos journal in March 2012.

```
Strange Beta: An Assistance System for Indoor Rock Climbing Route Setting Using Chaotic Variations and Machine Learning
Copyright (C) 2011  Caleb Philips, Lee Becker, Elizabeth Bradley

Modifications are made by Lindsay Kempen (2019)
xxxx-xx-xx Modified and documented existing code
xxxx-xx-xx Used parts of (altered) code in other files

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

### Variable-order Markov Model prediction

```
Copyright (C) 2004  Ron Begleiter, Ran El-Yaniv, Golan Yona.  
Code from the paper "On Prediction Using Variable Order Markov Models"  
BibTeX: http://www.cs.technion.ac.il/~rani/el-yaniv_bib.html#BegleiterEY04

Trivial modifications are made by Lindsay Kempen (2019)  
2019-01-18: Change formatting of vmm/algs/DCTWPredictor.java  
2019-01-17: Change encoding of files in the vmm folder to UTF-8

This code is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License (http://www.gnu.org/copyleft/gpl.html) for more details.
```

### Phoenix parser for spontaneous speech

```
Phoenix Spoken Language System is developed at Carnegie Mellon University.  
Paper: Ward, Wayne (1994): "Extracting information in spontaneous speech", In ICSLP-1994, 83-86.
```

### This project

```
A Fair Grade: Assessing Difficulty of Climbing Routes through Machine Learning  
Copyright (C) 2019  Lindsay Kempen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```


## FAQ
