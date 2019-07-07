/* HEADER
If you use this code don't forget to reference us :) BibTeX: http://www.cs.technion.ac.il/~rani/el-yaniv_bib.html#BegleiterEY04

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
GNU General Public License (<a href="http://www.gnu.org/copyleft/gpl.html">GPL</a>) for more details.*/

package vmm.algs;

import java.io.BufferedReader;
import java.io.Serializable;
import java.util.Iterator;
import java.util.Vector;
import java.util.HashMap;
import java.util.Set;

import vmm.pred.*;
import vmm.algs.decomp.*;
import vmm.util.*;

/**
 * <p>
 * Title: Decomposed CTW Predictor
 * </p>
 *
 * DCTWPredictor dctw = new DCTWPredictor(); dctw.init(256, 5);
 * dctw.learn("abracadabra"); System.out.println("logeval : " +
 * dctw.logEval("cadabra")); System.out.println("P(c|abra) : " +
 * dctw.predict('c', "abra"));
 *
 * <p>
 * Copyright: Copyright (c) 2004
 * </p>
 * 
 * @author <a href="http://www.cs.technion.ac.il">Ron Begleiter</a>
 * @version 1.0
 */

public class DCTWPredictor implements VMMPredictor, Serializable {

  private static final double NEGTIVE_INVERSE_LOG_2 = -(1 / Math.log(2.0));

  private StaticDecompositionNode dctw;
  private HashMap syms;

  private int abSize;
  private int vmmOrder;

  public DCTWPredictor() {
  }

  public void init(int abSize, int vmmOrder) {
    this.abSize = abSize;
    this.vmmOrder = vmmOrder;
    this.syms = new HashMap();
  }

  public Vector samplesFromInput(CharSequence trainingSequence) {
    Vector samples = new Vector();
    for (int i = 0, symbol = -1; i < trainingSequence.length(); ++i) {
      samples.add((int) trainingSequence.charAt(i));
    }
    return samples;
  }

  public Vector samplesFromInput(BufferedReader in) {
    Vector samples = new Vector();
    try {
      String line = in.readLine();
      while (line != null) {
        int symbol = Integer.parseInt(line);
        samples.add(symbol);
        line = in.readLine();
      }
    } catch (java.io.IOException e) {
      System.out.println(e);
    } catch (java.lang.NumberFormatException e) {
      System.out.println(e);
    }
    return samples;
  }

  public Set getSymbols() {
    return syms.keySet();
  }

  public void learn(Vector samples) {
    DecompositionTreeBuilder builder = new DecompositionTreeBuilder(abSize, vmmOrder);
    dctw = builder.buildStatic((SampleIterator) new VectorSampleIter(samples));

    Context context = new DefaultContext(vmmOrder);

    Iterator it = samples.iterator();
    while (it.hasNext()) {
      int symbol = (Integer) it.next();
      if (!syms.containsKey(symbol)) {
        syms.put(symbol, 1);
      } else {
        syms.put(symbol, ((Integer) syms.get(symbol)) + 1);
      }
      dctw.train(symbol, context);
      context.add(symbol);
    }
  }

  public void learn(CharSequence trainingSequence) {
    learn(samplesFromInput(trainingSequence));
  }

  public void learn(BufferedReader trainingSequence) {
    learn(samplesFromInput(trainingSequence));
  }

  public double predict(int symbol, BufferedReader context) {
    return predict(symbol, samplesFromInput(context));
  }

  public double predict(int symbol, CharSequence context) {
    return predict(symbol, samplesFromInput(context));
  }

  public double predict(int symbol, Vector context) {
    try {
      Iterator it = context.iterator();
      Context ctwContext = new DefaultContext(vmmOrder);
      while (it.hasNext()) {
        ctwContext.add((Integer) it.next());
      }
      return dctw.predict(symbol, ctwContext);
    } catch (NullPointerException npe) {
      if (dctw == null) {
        throw new VMMNotTrainedException();
      } else {
        throw npe;
      }
    }
  }

  public double logEval(Vector testSequence, Vector initialContext) {
    try {
      Context context = new DefaultContext(vmmOrder);
      Iterator it = initialContext.iterator();
      while (it.hasNext()) {
        context.add((Integer) it.next());
      }
      double eval = 0.0;
      it = testSequence.iterator();
      while (it.hasNext()) {
        int sym = (Integer) it.next();
        eval += Math.log(dctw.predict(sym, context));
        context.add(sym);
      }
      return eval * NEGTIVE_INVERSE_LOG_2;
    } catch (NullPointerException npe) {
      if (dctw == null) {
        throw new VMMNotTrainedException();
      } else {
        throw npe;
      }
    }
  }

  public double logEval(BufferedReader testSequence) {
    return logEval(samplesFromInput(testSequence));
  }

  public double logEval(Vector testSequence) {
    return logEval(testSequence, new Vector());
  }

  public double logEval(CharSequence testSequence) {
    return logEval(samplesFromInput(testSequence));
  }

  public double logEval(CharSequence testSequence, CharSequence initialContext) {
    return logEval(samplesFromInput(testSequence), samplesFromInput(initialContext));
  }
}
