# from astroquery.jplhorizons import Horizons
# import numpy as np

# earth = Horizons(id=399, epochs = {'start':'2005-06-20', 'stop':'2005-06-21','step':'1d'})
# earthVectors = earth.vectors()
# earthX = earthVectors['x'].data # X is in AU
# au2km = 149_597_870.7
# earthXkm = earthX * au2km # X is in km


# from astropy.coordinates import get_body_barycentric, solar_system_ephemeris
# from astropy.time import Time

# # set the ephemeris to use DE441
# solar_system_ephemeris.set("e:/data/jpl-de441/de441.bsp")

# t = Time("2005-06-20", scale="tdb")
# pos = get_body_barycentric("earth", t)

# print(pos.x)

from astropy.time import Time
from astropy.coordinates import solar_system_ephemeris, get_body_barycentric_posvel
from poliastro.bodies import Sun, SolarSystemBarycenter
from poliastro.ephem import Ephem
from poliastro.twobody import Orbit
import numpy as np

# Define the time for which you want to obtain the orbit parameters
time = Time.now()

# Set the solar system ephemeris
#solar_system_ephemeris.set('builtin')
solar_system_ephemeris.set('jpl') 

#print(solar_system_ephemeris.bodies)

# Get the position and velocity of Jupiter at the specified time
jupiter_position, jupiter_velocity = get_body_barycentric_posvel('jupiter', time)

# Compute orbital elements using poliastro
jupiter_orbit = Orbit.from_vectors(SolarSystemBarycenter, jupiter_position.xyz, jupiter_velocity.xyz, epoch=time)

# Print orbital elements
print("Semi-major Axis (a):", jupiter_orbit.a)
print("Eccentricity (e):", jupiter_orbit.ecc)
print("Inclination (i):", np.degrees(jupiter_orbit.inc), "degrees")
print("Longitude of Ascending Node (Ω):", np.degrees(jupiter_orbit.raan), "degrees")
print("Argument of Periapsis (ω):", np.degrees(jupiter_orbit.argp), "degrees")
print("True Anomaly (ν):", np.degrees(jupiter_orbit.nu), "degrees")
