import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.ObjectOutputStream;
import java.io.ObjectInputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;

import java.util.Vector;
import java.util.Iterator;
import java.util.Set;

import java.lang.String;

import vmm.algs.DCTWPredictor;
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

public class Test {
  public static void main(String args[]) {
    DCTWPredictor dctw = null;
    if (args.length < 2) {
      System.out.println("Usage: java -cp . Test learn model.ser [lang size] [model depth] < data.txt");
      System.out.println("       java -cp . Test predict|eval model.ser [symbol] < data.txt");
      System.out.println("       java -cp . Test smooth model.ser [before] [after]");
      return;
    }

    try {
      String action = args[0];
      String model = args[1];

      if (action.equals("learn")) {
        try {
          dctw = load(model);
        } catch (FileNotFoundException e) {
          dctw = new DCTWPredictor();
          int language_cardinality = 256;
          int model_depth = 5;
          if (args.length > 2) {
            language_cardinality = Integer.parseInt(args[2]);
          }
          if (args.length > 3) {
            model_depth = Integer.parseInt(args[3]);
          }
          dctw.init(language_cardinality, model_depth);
          System.out.println("Model file didn't exist");
        }
        dctw.learn(new BufferedReader(new InputStreamReader(System.in)));
        save(dctw, model);
      } else if (action.equals("predict")) {
        dctw = load(model);
        String symbol = args[2];
        BufferedReader context = new BufferedReader(new InputStreamReader(System.in));
        System.out.println(dctw.predict(Integer.parseInt(symbol), context));
      } else if (action.equals("eval")) {
        dctw = load(model);
        BufferedReader context = new BufferedReader(new InputStreamReader(System.in));
        System.out.println(dctw.logEval(context));
      } else if (action.equals("smooth")) {
        Vector before = csvToVector(args[2]);
        Vector after = csvToVector(args[3]);
        int maxAdd = Integer.parseInt(args[4]);
        dctw = load(model);
        Test t = new Test();
        Vector trans = t.smooth(dctw, before, after, maxAdd);
        StringBuffer sb = new StringBuffer();
        Iterator it = trans.iterator();
        while (it.hasNext()) {
          sb.append(it.next().toString() + " ");
        }
        System.out.println(sb);
      }
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public static void save(DCTWPredictor dctw, String fname) throws FileNotFoundException, IOException {
    FileOutputStream fos = new FileOutputStream(fname);
    ObjectOutputStream out = new ObjectOutputStream(fos);
    out.writeObject(dctw);
    out.close();
  }

  public static DCTWPredictor load(String fname) throws FileNotFoundException, IOException, ClassNotFoundException {
    FileInputStream fis = new FileInputStream(fname);
    ObjectInputStream in = new ObjectInputStream(fis);
    DCTWPredictor dctw = (DCTWPredictor) in.readObject();
    in.close();
    return dctw;
  }

  public static Vector csvToVector(String csv) {
    String[] parts = csv.split(",");
    Vector ret = new Vector();
    for (int i = 0; i < parts.length; i++) {
      ret.add(Integer.parseInt(parts[i]));
    }
    return ret;
  }

  public class SearchNode {
    public Vector v;
    public double badness;

    public SearchNode(Vector i) {
      this.v = i;
    }

    public double eval(DCTWPredictor model, Vector before, Vector after) {
      Vector full = new Vector(before);
      full.addAll(v);
      full.addAll(after);
      return model.logEval(full);
    }

    public int size() {
      return v.size();
    }

    public String toString() {
      return v.toString() + " => " + this.badness;
    }
  }

  // "minimizing the average log-loss is completely equivalent to maximizing a
  // probability assignment for the entire test sequence"
  // Begleiter et al. p. 387

  public SearchNode smooth(DCTWPredictor model, Vector before, SearchNode me, Vector after, int maxAdd) {

    // calculte my badness
    me.badness = me.eval(model, before, after);

    // System.out.println(me.toString());

    // assume I'm the best node
    SearchNode best = me;

    // compare myself to my children
    if (me.size() < maxAdd) {
      Set syms = model.getSymbols();
      Iterator it = syms.iterator();
      while (it.hasNext()) {
        int sym = (Integer) it.next();
        Vector child = new Vector(me.v);
        child.add(sym);
        SearchNode test = smooth(model, before, new SearchNode(child), after, maxAdd);
        if (test.badness < best.badness) {
          best = test;
        }
      }
    }

    return best;
  }

  public Vector smooth(DCTWPredictor model, Vector before, Vector after, int maxAdd) {
    SearchNode best = smooth(model, before, new SearchNode(new Vector()), after, maxAdd);
    return best.v;
  }

}
