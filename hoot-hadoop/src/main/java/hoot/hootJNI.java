/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.40
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package hoot;

class hootJNI {
  public final static native String PbfInputFormat_className();
  public final static native long new_PbfInputFormat();
  public final static native long PbfInputFormat_getSplit(long jarg1, PbfInputFormat jarg1_, int jarg2);
  public final static native int PbfInputFormat_getSplitCount(long jarg1, PbfInputFormat jarg1_);
  public final static native void PbfInputFormat_setConfiguration(long jarg1, PbfInputFormat jarg1_, long jarg2);
  public final static native void delete_PbfInputFormat(long jarg1);
  public final static native long new_PbfInputSplit();
  public final static native int PbfInputSplit_getLength(long jarg1, PbfInputSplit jarg1_);
  public final static native String PbfInputSplit_getLocations(long jarg1, PbfInputSplit jarg1_);
  public final static native int PbfInputSplit_getStart(long jarg1, PbfInputSplit jarg1_);
  public final static native void PbfInputSplit_readFields(long jarg1, PbfInputSplit jarg1_, String jarg2, int jarg3);
  public final static native void PbfInputSplit_setLocations(long jarg1, PbfInputSplit jarg1_, String jarg2);
  public final static native void PbfInputSplit_setLength(long jarg1, PbfInputSplit jarg1_, int jarg2);
  public final static native void PbfInputSplit_setStart(long jarg1, PbfInputSplit jarg1_, int jarg2);
  public final static native long PbfInputSplit_writeFields(long jarg1, PbfInputSplit jarg1_, long jarg2);
  public final static native void delete_PbfInputSplit(long jarg1);
}