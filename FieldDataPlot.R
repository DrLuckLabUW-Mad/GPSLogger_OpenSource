# these are packages you will need, but probably already have.
# Don't bother installing if you already have them
install.packages(c("ggplot2", "devtools", "dplyr", "stringr"))

# some standard map packages.
install.packages(c("maps", "mapdata"))

# the github version of ggmap
devtools::install_github("dkahle/ggmap")

# includes the packages that were installed earlier so they can be used in the code
library(ggplot2)
library(ggmap)
library(maps)
library(mapdata)

#Reads the CSV files, indicates where to find the file, and that it has headers
#Make sure the code in "" matches where the files are actually found
fielddata <- read.csv("C:/Users/Claire/Desktop/GPS Logs/GPSLOG40.csv", header = T)

# Get your own API key from google to open the satellite map
register_google(key="AIzaSyAYD3QMmTzXLRCRDK1TsdY0HWOtU_GQHZo")


#Indicates the max and min lon and lat to create a box for the map data and prints it out
#If the following message shows up "Warning message: Removed XXX rows containing missing 
#values (geom_point)."
#Change 'f' value to make sure all of the data fits within the box.
FieldCoord <- make_bbox(lon = fielddata$longitude, lat = fielddata$latitude, f = .01)
FieldCoord

#Makes a satellite map from google with the same coordinates as the FieldCoord
Field_map <- get_map(location = FieldCoord, maptype = "satellite", source = "google")
Field_map

#Plots the map on a separate screen with the longitude nd latitude data from the cSV file
#Each line from the CSv file is indicated with a point
X11()
ggmap(Field_map) + geom_point(data=fielddata, mapping = aes(x = longitude, y = latitude), color = "red")

