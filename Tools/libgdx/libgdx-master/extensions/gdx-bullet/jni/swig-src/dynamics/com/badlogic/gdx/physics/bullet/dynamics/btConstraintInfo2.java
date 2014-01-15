/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.10
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.badlogic.gdx.physics.bullet.dynamics;

import com.badlogic.gdx.physics.bullet.BulletBase;
import com.badlogic.gdx.physics.bullet.linearmath.*;
import com.badlogic.gdx.physics.bullet.collision.*;
import com.badlogic.gdx.math.Vector3;
import com.badlogic.gdx.math.Quaternion;
import com.badlogic.gdx.math.Matrix3;
import com.badlogic.gdx.math.Matrix4;

public class btConstraintInfo2 extends BulletBase {
	private long swigCPtr;
	
	protected btConstraintInfo2(final String className, long cPtr, boolean cMemoryOwn) {
		super(className, cPtr, cMemoryOwn);
		swigCPtr = cPtr;
	}
	
	/** Construct a new btConstraintInfo2, normally you should not need this constructor it's intended for low-level usage. */ 
	public btConstraintInfo2(long cPtr, boolean cMemoryOwn) {
		this("btConstraintInfo2", cPtr, cMemoryOwn);
		construct();
	}
	
	@Override
	protected void reset(long cPtr, boolean cMemoryOwn) {
		if (!destroyed)
			destroy();
		super.reset(swigCPtr = cPtr, cMemoryOwn);
	}
	
	public static long getCPtr(btConstraintInfo2 obj) {
		return (obj == null) ? 0 : obj.swigCPtr;
	}

	@Override
	protected void finalize() throws Throwable {
		if (!destroyed)
			destroy();
		super.finalize();
	}

  @Override protected synchronized void delete() {
		if (swigCPtr != 0) {
			if (swigCMemOwn) {
				swigCMemOwn = false;
				DynamicsJNI.delete_btConstraintInfo2(swigCPtr);
			}
			swigCPtr = 0;
		}
		super.delete();
	}

  public void setFps(float value) {
    DynamicsJNI.btConstraintInfo2_fps_set(swigCPtr, this, value);
  }

  public float getFps() {
    return DynamicsJNI.btConstraintInfo2_fps_get(swigCPtr, this);
  }

  public void setErp(float value) {
    DynamicsJNI.btConstraintInfo2_erp_set(swigCPtr, this, value);
  }

  public float getErp() {
    return DynamicsJNI.btConstraintInfo2_erp_get(swigCPtr, this);
  }

  public void setJ1linearAxis(SWIGTYPE_p_float value) {
    DynamicsJNI.btConstraintInfo2_J1linearAxis_set(swigCPtr, this, SWIGTYPE_p_float.getCPtr(value));
  }

  public SWIGTYPE_p_float getJ1linearAxis() {
    long cPtr = DynamicsJNI.btConstraintInfo2_J1linearAxis_get(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public void setJ1angularAxis(SWIGTYPE_p_float value) {
    DynamicsJNI.btConstraintInfo2_J1angularAxis_set(swigCPtr, this, SWIGTYPE_p_float.getCPtr(value));
  }

  public SWIGTYPE_p_float getJ1angularAxis() {
    long cPtr = DynamicsJNI.btConstraintInfo2_J1angularAxis_get(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public void setJ2linearAxis(SWIGTYPE_p_float value) {
    DynamicsJNI.btConstraintInfo2_J2linearAxis_set(swigCPtr, this, SWIGTYPE_p_float.getCPtr(value));
  }

  public SWIGTYPE_p_float getJ2linearAxis() {
    long cPtr = DynamicsJNI.btConstraintInfo2_J2linearAxis_get(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public void setJ2angularAxis(SWIGTYPE_p_float value) {
    DynamicsJNI.btConstraintInfo2_J2angularAxis_set(swigCPtr, this, SWIGTYPE_p_float.getCPtr(value));
  }

  public SWIGTYPE_p_float getJ2angularAxis() {
    long cPtr = DynamicsJNI.btConstraintInfo2_J2angularAxis_get(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public void setRowskip(int value) {
    DynamicsJNI.btConstraintInfo2_rowskip_set(swigCPtr, this, value);
  }

  public int getRowskip() {
    return DynamicsJNI.btConstraintInfo2_rowskip_get(swigCPtr, this);
  }

  public void setConstraintError(SWIGTYPE_p_float value) {
    DynamicsJNI.btConstraintInfo2_constraintError_set(swigCPtr, this, SWIGTYPE_p_float.getCPtr(value));
  }

  public SWIGTYPE_p_float getConstraintError() {
    long cPtr = DynamicsJNI.btConstraintInfo2_constraintError_get(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public void setCfm(SWIGTYPE_p_float value) {
    DynamicsJNI.btConstraintInfo2_cfm_set(swigCPtr, this, SWIGTYPE_p_float.getCPtr(value));
  }

  public SWIGTYPE_p_float getCfm() {
    long cPtr = DynamicsJNI.btConstraintInfo2_cfm_get(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public void setLowerLimit(SWIGTYPE_p_float value) {
    DynamicsJNI.btConstraintInfo2_lowerLimit_set(swigCPtr, this, SWIGTYPE_p_float.getCPtr(value));
  }

  public SWIGTYPE_p_float getLowerLimit() {
    long cPtr = DynamicsJNI.btConstraintInfo2_lowerLimit_get(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public void setUpperLimit(SWIGTYPE_p_float value) {
    DynamicsJNI.btConstraintInfo2_upperLimit_set(swigCPtr, this, SWIGTYPE_p_float.getCPtr(value));
  }

  public SWIGTYPE_p_float getUpperLimit() {
    long cPtr = DynamicsJNI.btConstraintInfo2_upperLimit_get(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_float(cPtr, false);
  }

  public void setFindex(SWIGTYPE_p_int value) {
    DynamicsJNI.btConstraintInfo2_findex_set(swigCPtr, this, SWIGTYPE_p_int.getCPtr(value));
  }

  public SWIGTYPE_p_int getFindex() {
    long cPtr = DynamicsJNI.btConstraintInfo2_findex_get(swigCPtr, this);
    return (cPtr == 0) ? null : new SWIGTYPE_p_int(cPtr, false);
  }

  public void setNumIterations(int value) {
    DynamicsJNI.btConstraintInfo2_numIterations_set(swigCPtr, this, value);
  }

  public int getNumIterations() {
    return DynamicsJNI.btConstraintInfo2_numIterations_get(swigCPtr, this);
  }

  public void setDamping(float value) {
    DynamicsJNI.btConstraintInfo2_damping_set(swigCPtr, this, value);
  }

  public float getDamping() {
    return DynamicsJNI.btConstraintInfo2_damping_get(swigCPtr, this);
  }

  public btConstraintInfo2() {
    this(DynamicsJNI.new_btConstraintInfo2(), true);
  }

}
