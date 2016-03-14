package com.rt.WAXBlue;

import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.concurrent.Semaphore;

/**
 * Author: Rob Thompson
 * Date: 24/03/2014
 */
public class Writer extends Thread {

    private static final String TAG = "Writer Thread";     //Log Tag
    private static final boolean D = true;                 //Log flag, set to true to print debug messages
    private volatile FileOutputStream fos;                 //File output stream used to write.
    private volatile LinkedList<byte[]> bigBuffer;         //Linked list containing byte[] buffers with data from device
    private volatile LinkedList<Integer> sizes;            //Linked list containing sizes of byte[] buffers
    private boolean isRunning = true;                      //Flag to keep run loop going.

    /**
     *
     * @param file      File to write to.
     * @param bigBuffer Linked list containing byte[] buffers with data from device
     * @param sizes     Linked list containing sizes of byte[] buffers
     */
    public Writer(File file, LinkedList<byte[]> bigBuffer, LinkedList<Integer> sizes) {

        if(D) Log.d(TAG, "Creating new Writer Thread");

        //Create file output stream to write with
        try {
            fos = new FileOutputStream(file, true);
        } catch (IOException e) {
            Log.e(TAG, "Failed to created Buffered Writer");
        }


        this.bigBuffer = bigBuffer;                            //Linked list containing data to be written
        this.sizes = sizes;                                    //Linked list containing size information about data.
    }

    @Override
    public synchronized void run() {

        if (D) Log.d(TAG, "Running Writer Thread");

//        try {
//            allDone.acquire();
//        } catch (InterruptedException e) {
//            Log.e(TAG, "Failed to acquire semaphore");
//        }
        //Write timestamp to data.
        long time = System.currentTimeMillis();
        try {
            fos.write((time + "\r\n").getBytes());
        } catch (IOException e) {
            Log.e(TAG, "Write Error");
        }

        //Loop until told to stop.
        while (isRunning) {

            // Wait until we have data
            try {
                wait();
            } catch (InterruptedException ignored) {
            }

            // Write all the data we have
            for (; ; ) {


                byte[] buffer = null;
                int size = 0;

                // Safely get the next thing to write and its size
                synchronized (bigBuffer) {
                    if (bigBuffer.size() > 0) {
                        buffer = bigBuffer.removeLast();
                        size = sizes.removeLast();
                    }
                }


                // Break out when we have no more data
                if (buffer == null) {
                    break;
                }

                //Trim buffer to remove null values
                buffer = Arrays.copyOf(buffer, size);

                //write the buffer contents to the file
                try {
                    fos.write(buffer);
                } catch (IOException e2) {
                    Log.e(TAG, "Failed Writing to file: " + e2.getMessage());
                }

            }

        }

        //Write timestamp when finished.
        time = System.currentTimeMillis();
        try {

            fos.write(("\r\n" + time).getBytes());
        } catch (IOException e) {
            Log.e(TAG, "Write Error", e);
        }

        //close the file output stream
        try {
            fos.close();
        } catch (IOException e) {
            Log.e(TAG, "Failed to closed file output stream", e);
        }

        //allDone.release();
    }

    /**
     * Called when the stop button is press. Shuts down the writer thread.
     */
    public synchronized void shutdown() {

        if (D) Log.d(TAG, "Stopping Writer Thread: " + getName());

        //stop run loop
        isRunning = false;

    }
}
