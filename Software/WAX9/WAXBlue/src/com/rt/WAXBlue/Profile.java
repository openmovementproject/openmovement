package com.rt.WAXBlue;

import java.io.Serializable;

/**
 * Author: Rob Thompson
 * Date: 16/04/2014
 */
public class Profile implements Serializable{

    private String name;
    private String[] locations;

    public Profile(String name, String[] locations) {
        this.name = name;
        this.locations = locations;
    }

    public Profile(){
        this.name = "";
        this.locations = null;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String[] getLocations() {
        return locations;
    }

    public void setLocations(String[] locations) {
        this.locations = locations;
    }
}
