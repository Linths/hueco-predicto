/* HEADER 
If you use this code don’t forget to reference us :) BibTeX: http://www.cs.technion.ac.il/~rani/el-yaniv_bib.html#BegleiterEY04 

This code is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License 
as published by the Free Software Foundation; either version 2 
of the License, or (at your option) any later version. 

This code is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
GNU General Public License (<a href="http://www.gnu.org/copyleft/gpl.html">GPL</a>) for more details.*/ 
 
package vmm.util;

import vmm.util.SampleIterator;

import java.util.Vector;
import java.util.Iterator;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2003</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */

public class VectorSampleIter implements SampleIterator {
  private static final String NULL_NAME = "*NONAME*";
  private Iterator it;
  private Vector data;
  private int dataInd;

  private String name;

  public VectorSampleIter(Vector v) {
    this.it = v.iterator();
    this.data = v;
    name = NULL_NAME;
  }

  public boolean hasNext() { return it.hasNext(); }

  public int next() { return (Integer)it.next(); }

  public void restart () { this.it = this.data.iterator(); }

  public long size() { return data.size(); }

  public void setName(String name) { this.name = name; }

  public String getName() { return name; }

}
