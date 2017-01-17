# DoorD
A simple server daemon used to control the state of a garage door.

## About
Having grown tired of purchasing universal garage door remotes and having them break, I decided the best solution would be to create a
system where my phone could act as the remote to my garage door. The entire system is composed of two platforms: an Android based cell
phone, and wireless router running [OpenWrt](https://openwrt.org) with a gpio pin broken out. The wireless router could be replaced with
almost any embedded linux system (RaspberryPi, BeagleBone), I just happend to have a spare router on hand.

The router is electrically connected to the garage door motor and acts as the radio interface to the associated Android application. The
Android application acts as the user interface and allows the garage door to be opened, stopped, or closed. 
