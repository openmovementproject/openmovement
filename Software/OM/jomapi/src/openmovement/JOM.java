/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */

// Java Open Movement
// Dan Jackson, 2012

package openmovement;

import java.util.Date;

public class JOM {

	/** Open Movement API singleton instance */
	private static JOM instance;

	/** Get the Open Movement API singleton instance */
	public static JOM getInstance()
	{
		if (instance == null) { instance = new JOM(); }
		return instance;
	}

	/** Singleton constructor */
	private JOM() {
		System.out.println("JOM: OmStartup()");
		JOMAPI.OmStartup(JOMAPI.OM_VERSION);
		return;
	}

	/** Singleton finalizer */
	protected void finalize() throws Throwable {
		try {
			System.out.println("JOM: OmShutdown()");
			JOMAPI.OmShutdown();
		} finally {
			super.finalize();
		}
	}
	
	/** Demonstration function */
	public void demo() {
		// NOTE: This is not the recommended method for finding devices, should use a callback instead
		int[] deviceIds = new int[0x10000];
		int numDevices = JOMAPI.OmGetDeviceIds(deviceIds, deviceIds.length);
		for (int i = 0; i < numDevices; i++) {
			System.out.println("#" + i + " found device");
		}
	}
	
	/** Main for demonstration function */
	public static void main(String[] args) {
		System.out.println("JOM: Demo started.");
		JOM om = JOM.getInstance();
		om.demo();
		System.out.println("JOM: Demo ended.");
	}

}
