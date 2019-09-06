# :mount_fuji:Hueco Predicto:crystal_ball:

This is a climbing difficulty classifier, developed as part of a research paper.  
:page_facing_up: _A fair grade: assessing difficulty of climbing routes through machine learning_ (Lindsay Kempen, 2019)  
:books: Full text via [bachelorthesis.linths.com](http://bachelorthesis.linths.com)

All data used in this work (in-kind and anonymized), and some of the underlying code, was provided  by the [Strange Beta](http://strangebeta.com) project by [Caleb Phillips](http://smallwhitecube.com).

[Send me](mailto:code@linths.com) your questions, opinions and ideas!


## Paper abstract

In the sport of climbing and bouldering, difficulty grades are important for skill assessment, enjoyment and safety. Although there are standardized scales to minimize inconsistency, the difficulty assessment is performed by humans and sensitive to subjectivity. Therefore, we have developed a tool that predicts the difficulty grades of climbing routes using machine-learning.  
The routes are described in a semi-natural Domain-Specific Language, which can be parsed into symbol sequences. Here, a symbol represents a climbing move. The symbol sequences are then used as inputs to a variable-order Markov models (VOMMs) based classifier. With the VOMM prediction algorithm Decomposed Context Tree Weighting (DE-CTW), we trained one VOMM on _Easy_ climbing routes and one on _Hard_ climbing routes. By calculating a test route's likelihood for both VOMMs, the _average log-loss_}, we predict if a route is _Easy_ or _Hard_.  
We have implemented six predictor variations to vary with interpretation detail in the symbolization process. After using 50-fold cross validation on 146 climbing routes, our best performing variation performed slightly better than a trivial classifier. Still we believe this research's foundations are of interest for future research. We conclude with detailed explanations and proposed improvements.

Full text: [bachelorthesis.linths.com](http://bachelorthesis.linths.com)

## Run it

Recommended settings
- **Model depth = 5**  
Used for the VOMM learning process. This is the max length of subsequences (_context_, here: number of consecutive climbing holds/moves) considered for the probability calculation.
- **Identifier**  
Determines save location for VOMMs (`vomm/src/data/grades/<identifier>`). A program run erases the existing folder contents first, so use a new ID if you want to keep the old contents.
- **k ∈ {10..30}**  
A high k can take hours, but it can benefit the accuracy with this small of a dataset.
- **N <= data_size**  
A high N can take hours. Choose a N of -1 to set N to data_size.


Commands  
First run? Run `make` in [`vomm/src`](vomm/src) first.
- **k-fold cross-validation** :clock1::clock1::clock1:  
  Performs k-fold cross-validation. Classifies climbing routes in 6 variations. Outputs classifications and confusion matrices for every variation.  
  `ruby performance_kfold.rb <k> <model-depth> <id>`
- **Classify a random route** :clock1::clock1:  
  Performs N rounds of classification, taking a random climbing route as test set and all other routes as train set. Routes cannot be picked more than once. Classifies in 6 variations. Outputs classifications and confusion matrices for every variation.  
  `ruby performance_one.rb <N> <model-depth> <id>`
- **Only train on all routes** :clock1:  
  `vomm_train_all.sh`
- **Play with patterns** :zap:  
  See DE-CTW's magic of pattern recognition. Run `playground.sh` and `test.sh` in [`vomm/src`](vomm/src) for quick and simple examples. You can easily adjust the examples.

## Tweak it
For your ease of navigation, every filename in this doc is a clickable link.

- **Change grade classes**  
    Edit [`grades.rb`](grades.rb). It already contains alternative grade class distributions.
    - Separate the climbing and bouldering routes: set `MergeMode` to false.
    - Choose other distributions: assign them to
    `boulder_conversion` & `climb_conversion` (if `MergeMode` is true) or `boulder_classes` and `climb_classes` (if `MergeMode` is false).
- **Change grammar**  
    Edit [`climbing.gra`](phoenix/app/strangebeta/Grammar/climbing.gra) and parse afterwards:
    1. Copy the `climbing.gra` file right outside the Grammar folder. (A weird but necessary hack.)
    2. Run [`./run_grammar`](phoenix/app/strangebeta/run_grammar)
    3. Drag the [`base.dic`](phoenix/app/strangebeta/Grammar/base.dic) and [`simple.net`](phoenix/app/strangebeta/Grammar/simple.net) to the Grammar folder, replace the old files
    4. Parse by running `make` in [`phoenix/app/strangebeta/`](phoenix/app/strangebeta/)
- **Change symbolization**  
    1. You can alter the symbol sets by changing how symbols are made from parsed climbing routes. In [`move.rb`](phoenix/app/strangebeta/move.rb), [`resymbolize.rb`](phoenix/app/strangebeta/resymbolize.rb) and [`symbolize.rb`](phoenix/app/strangebeta/symbolize.rb) you can determine the influence of aspects like hold shape, hold size, action shape, etc.
    2. Run `make` in [`phoenix/app/strangebeta/`](phoenix/app/strangebeta/)
- **Change data (sanitation)**  
    1. Edit the input data in [`strangebeta_user_climb_data_20180128.txt`](data/strangebeta_user_climb_data_20180128.txt) and/or edit the pre-parser sanitation in [`sb_ucd_anal.rb`](data/sb_ucd_anal.rb).
    3. Run `make` in [`phoenix/app/strangebeta/`](phoenix/app/strangebeta/)

## Code structure

- `vomm`: Variable-order Markov model prediction
  - `vmm` & `doc`: Code & documentation  
  :copyright: Begleiter et al.
  - `data`: VOMMs generated using this code, using the climbing route input
- `phoenix`: Parser
  - `PhoenixLib` & `Doc`: Code & documentation.  
  :copyright: Wayne Ward / CMU
  - `app/strangebeta`: Parser grammar, input, output and extra parsing steps (e.g. input sanitation, symbolization).  
  :copyright: Philips et al. Modifications were made.
- `data`: Input data  
  :copyright: Philips et al. Modifications were made.
- `output`: Output logs

## Interesting files

You can familiarize yourself more with the program by glancing over:
- [`human.txt`](phoenix/app/strangebeta/human.txt) - Human-readable versions of the four symbol sets. Columns: symbol set, symbol, climbing move.
- [`climbing.gra`](phoenix/app/strangebeta/Grammar/climbing.gra) - Parser grammar
- [`public_moves.csv`](public_moves.csv) - Readable climbing data. *Note: actual input data is [elsewhere](data/strangebeta_user_climb_data_20180128.txt).*

## Copyright

### Strange Beta: a climbing route assistance system

The Strange Beta system is a large contribution to this project. The Strange Beta components included in this project are listed below.. Several files have been modified and (adapted) parts have been reused for new files.

- Data: climbing routes expressed in a semi-natural DSL
- Climbing route generator (_not used and removed, traces of it may exist_)
- Climbing route parsing & symbolization
- Climbing route interpolation using variable-order Markov models

For more information on [Strange Beta](http://strangebeta.com), please reference "[strange beta: an assistance system for indoor rock climbing route setting](https://aip.scitation.org/doi/10.1063/1.3693047)" published in the AIP Chaos journal in March 2012.

```
Strange Beta: An Assistance System for Indoor Rock Climbing Route Setting
Copyright (C) 2011  Caleb Philips, Lee Becker, Elizabeth Bradley
Website: http://strangebeta.com
Article: https://aip.scitation.org/doi/10.1063/1.3693047

Modifications are made by Lindsay Kempen (2019)
2019 January & February  Modified and documented existing code
2019 January & February  Used parts of (altered) code in other files

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
